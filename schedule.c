/**
 * @file schedule.c
 * @brief 任务调度层：纯软业务逻辑装配车间 (基于 Pub/Sub & OOP 零耦合)
 *        (已根据用户要求裁撤测速反馈模块)
 */
#include "schedule.h"
#include "Application/broker/broker.h"
#include "Application/framework/simple_task_scheduler.h"
#include "Application/vofa/vofa.h"
#include "Device/track/track.h"
#include "Device/jy61p/jy61p.h"
#include "Application/algorithm/inertial_nav.h"
#include "Mode/mode_manager.h"

/* 彻底杜绝 extern 声明污染，从统一源取得底层映射与实例化本体 */
#include "extern.h"
#include "BSP/bsp_uart.h"
#include "BSP/bsp_timer.h"
#include "BSP/bsp_pwm.h"
#include "BSP/bsp_tb6612.h"
#include "BSP/bsp_encoder.h"
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ti_msp_dl_config.h"
#include "Application/algorithm/pid_controller.h"

/************************** 局部全局状态防溢出 **************************/
Track_Instance g_track;
static bool  g_yaw_calibrated = false;
static float g_yaw_offset     = 0.0f;
static float g_yaw_current    = 0.0f;

/* 编码器实例定义 */
MotorEncoder_t g_enc_left;
MotorEncoder_t g_enc_right;

/* 速度闭环控制 */
typedef enum { SPEED_CTRL_OPEN = 0, SPEED_CTRL_CLOSED } SpeedCtrlMode_e;
static SpeedCtrlMode_e s_speed_ctrl_mode = SPEED_CTRL_OPEN;
static PIDInstance s_pid_speed_left;
static PIDInstance s_pid_speed_right;

/* ============== 速度斜坡限制器 ==============
 * 强制 |dv/dt| ≤ SQUARE_RAMP_ACCEL (mm/s^2)
 * 插在 Mode 输出 (TARGET_SPEED) 与速度 PID 之间，所有运动模式统一受保护。
 * 状态在 5ms 控制环里更新，发布至 TOPIC_TARGET_SPEED_RAMP_* 用于调试。 */
static float s_ramp_left  = 0.0f;
static float s_ramp_right = 0.0f;

static inline float ramp_step(float current, float target, float max_delta)
{
    float delta = target - current;
    if (delta >  max_delta) delta =  max_delta;
    if (delta < -max_delta) delta = -max_delta;
    return current + delta;
}

/* Vofa / BT 适配投递接口 */
static void bt_send_adapter(const uint8_t *data, uint16_t len) { BSP_BT_SendPacket((uint8_t *)data, len); }
static void track_send_adapter(uint8_t *data, uint16_t len) { BSP_Track_SendPacket(data, len); }

uint16_t ST_GetTimerTick(void) { return BSP_Timer_GetScheduleTick(); }

/************************** 硬件适配挂脚 (只存活于 Init 装配图) **************************/
static void _hw_left_set_dir(int8_t dir) { BSP_TB6612_SetDirLeft(dir); }
static void _hw_right_set_dir(int8_t dir) { BSP_TB6612_SetDirRight(-dir); }
static void _hw_left_set_pwm(uint32_t duty) { BSP_PWM_SetDuty(PWM_CHANNEL_0, duty, 1000); }
static void _hw_right_set_pwm(uint32_t duty) { BSP_PWM_SetDuty(PWM_CHANNEL_1, duty, 1000); }

/************************** 收发解析逻辑 (Broker交互) **************************/
#define BT_MAX_PACKET_LEN 128
static volatile char     s_bt_rx_buf[BT_MAX_PACKET_LEN];
static volatile uint16_t s_bt_rx_len   = 0;
static volatile bool     s_bt_rx_ready = false;

void BSP_BT_ProcessPacket(uint8_t *data, uint16_t len) {
    for (uint16_t i = 0; i < len; i++) {
        char c = (char)data[i];
        if (c == '\n' || c == '\r') {
            if (s_bt_rx_len > 0) {
                s_bt_rx_buf[s_bt_rx_len] = '\0';
                s_bt_rx_ready = true;
            }
        } else {
            if (!s_bt_rx_ready && s_bt_rx_len < BT_MAX_PACKET_LEN - 1) {
                s_bt_rx_buf[s_bt_rx_len++] = c;
            }
        }
    }
}

static void System_Parse_BT(void) {
    if (!s_bt_rx_ready) return;

    char local_buf[BT_MAX_PACKET_LEN];
    uint16_t local_len = s_bt_rx_len;
    memcpy(local_buf, (const char *)s_bt_rx_buf, local_len + 1);

    s_bt_rx_len   = 0;
    s_bt_rx_ready = false;

    /* ========== 任务启动指令 T<n>（当前唯一任务：矩形循迹） ========== */
    if (local_buf[0] == 'T' || local_buf[0] == 't') {
        int task_num = (int)strtol(local_buf + 1, NULL, 10);
        char ack_buf[64];
        int ack_len = 0;

        if (task_num == 1) {
            Mode_SetTask((uint8_t)task_num);
            ack_len = snprintf(ack_buf, sizeof(ack_buf),
                "ACK:T1 SQUARE STARTED\r\n");
        } else {
            ack_len = snprintf(ack_buf, sizeof(ack_buf),
                "ERR:UNKNOWN T%d\r\n", task_num);
        }
        if (ack_len > 0) {
            BSP_BT_SendPacket((uint8_t *)ack_buf, (uint16_t)ack_len);
        }
        return;
    }

    /* ========== 速度闭环指令 S<left>[,<right>] (mm/s) ========== */
    if (local_buf[0] == 'S' || local_buf[0] == 's') {
        char *p = local_buf + 1;
        float spd_l = (float)strtod(p, &p);
        float spd_r = spd_l;
        if (*p == ',') { p++; spd_r = (float)strtod(p, &p); }

        s_speed_ctrl_mode = SPEED_CTRL_CLOSED;
        if (Mode_GetMode() != CAR_MODE_IDLE) { Mode_SetIdle(); }
        Broker_Publish_Float(TOPIC_TARGET_WHEEL_SPEED_LEFT,  spd_l);
        Broker_Publish_Float(TOPIC_TARGET_WHEEL_SPEED_RIGHT, spd_r);

        /* 目标归零时清空 PID 积分，防止残留输出拖慢制动 */
        if (spd_l == 0.0f && spd_r == 0.0f) {
            PIDInit(&s_pid_speed_left,  &(PID_Init_Config_s){ .Kp=PID_WHEEL_KP, .Ki=PID_WHEEL_KI, .Kd=PID_WHEEL_KD, .MaxOut=PID_WHEEL_MAX_OUT, .IntegralLimit=PID_WHEEL_INTEGRAL_LIMIT, .DeadBand=PID_WHEEL_DEADBAND, .Improve=PID_Integral_Limit });
            PIDInit(&s_pid_speed_right, &(PID_Init_Config_s){ .Kp=PID_WHEEL_KP, .Ki=PID_WHEEL_KI, .Kd=PID_WHEEL_KD, .MaxOut=PID_WHEEL_MAX_OUT, .IntegralLimit=PID_WHEEL_INTEGRAL_LIMIT, .DeadBand=PID_WHEEL_DEADBAND, .Improve=PID_Integral_Limit });
        }

        char ack[80];
        int alen = snprintf(ack, sizeof(ack),
            "ACK:S L=%.0f R=%.0f mm/s\r\n", spd_l, spd_r);
        if (alen > 0) BSP_BT_SendPacket((uint8_t *)ack, (uint16_t)alen);
        return;
    }

    /* ========== 优先检测模式指令 M<n>[,p1][,p2] ========== */
    if (local_buf[0] == 'M' || local_buf[0] == 'm') {
        int mode_num = -1;
        float param1 = 0.0f, param2 = 0.0f;
        char ack_buf[80];
        int ack_len = 0;

        /* 解析: M<n>,<p1>,<p2>  (逗号分隔, 参数可选) */
        char *p = local_buf + 1;
        mode_num = (int)strtol(p, &p, 10);
        if (*p == ',') { p++; param1 = (float)strtod(p, &p); }
        if (*p == ',') { p++; param2 = (float)strtod(p, &p); }

        switch (mode_num) {
            case 0:  /* M0 → 空闲 + 恢复开环 */
                Mode_SetIdle();
                s_speed_ctrl_mode = SPEED_CTRL_OPEN;
                ack_len = snprintf(ack_buf, sizeof(ack_buf), "ACK:M0 IDLE\r\n");
                break;

            case 1: { /* M1,<pwm>[,<yaw>] → 直线 (yaw=0 自动锁当前航向) */
                float lock_yaw = param2;
                if (lock_yaw == 0.0f) {
                    lock_yaw = Broker_Subscribe_Float(TOPIC_JY61P_YAW);
                }
                Mode_SetStraight(param1, lock_yaw);
                ack_len = snprintf(ack_buf, sizeof(ack_buf),
                    "ACK:M1 STRAIGHT pwm=%.0f yaw=%.1f\r\n", param1, lock_yaw);
                break;
            }

            case 2:  /* M2,<max_pwm> → 循迹 */
                Mode_SetTracking(param1);
                ack_len = snprintf(ack_buf, sizeof(ack_buf),
                    "ACK:M2 TRACKING max=%.0f\r\n", param1);
                break;

            case 3:  /* M3,<target_yaw> → 原地转向 */
                Mode_SetTurning(param1);
                ack_len = snprintf(ack_buf, sizeof(ack_buf),
                    "ACK:M3 TURNING yaw=%.1f\r\n", param1);
                break;

            case 4:  /* M4,<pwm>,<yaw> → 弧线 */
                Mode_SetCurve(param1, param2);
                ack_len = snprintf(ack_buf, sizeof(ack_buf),
                    "ACK:M4 CURVE pwm=%.0f yaw=%.1f\r\n", param1, param2);
                break;

            case 5:  /* M5 → 制动 */
                Mode_SetBrake();
                ack_len = snprintf(ack_buf, sizeof(ack_buf), "ACK:M5 BRAKE\r\n");
                break;

            case 6:  /* M6 → 完成 */
                Mode_SetDone();
                ack_len = snprintf(ack_buf, sizeof(ack_buf), "ACK:M6 DONE\r\n");
                break;

            default:
                ack_len = snprintf(ack_buf, sizeof(ack_buf),
                    "ERR:UNKNOWN M%d\r\n", mode_num);
                break;
        }

        if (ack_len > 0) {
            BSP_BT_SendPacket((uint8_t *)ack_buf, (uint16_t)ack_len);
        }
        return;  /* 模式指令处理完毕，不走 L/R 逻辑 */
    }

    /* ========== 向后兼容：L<val>R<val> 直接 PWM 指令 ========== */
    char *ptr_l = strchr(local_buf, 'L');
    char *ptr_r = strchr(local_buf, 'R');

    float target_l = Broker_Subscribe_Float(TOPIC_TARGET_SPEED_LEFT);
    float target_r = Broker_Subscribe_Float(TOPIC_TARGET_SPEED_RIGHT);

    if (ptr_l || ptr_r) {
        if (ptr_l) {
            ptr_l++;
            while (*ptr_l == ':' || *ptr_l == '=' || *ptr_l == ' ') ptr_l++;
            target_l = (float)atof(ptr_l);
        }
        if (ptr_r) {
            ptr_r++;
            while (*ptr_r == ':' || *ptr_r == '=' || *ptr_r == ' ') ptr_r++;
            target_r = (float)atof(ptr_r);
        }
    } else {
        float val = (float)atof(local_buf);
        target_l = val;
        target_r = val;
    }

    /* L/R 指令自动切入 IDLE 模式，防止 Mode_Exec 覆盖 PWM */
    if (Mode_GetMode() != CAR_MODE_IDLE) {
        Mode_SetIdle();
    }

    Broker_Publish_Float(TOPIC_TARGET_SPEED_LEFT, target_l);
    Broker_Publish_Float(TOPIC_TARGET_SPEED_RIGHT, target_r);
}

void BSP_WIT_ProcessPacket(uint8_t *data, uint16_t len) {
    for (uint16_t i = 0; i < len; i++) {
        JY61P_FIFO_Write(&g_jy61p, data[i]);
    }
}

#define TRACK_RX_BUF_SIZE 128
static volatile uint8_t  s_track_rx_buf[TRACK_RX_BUF_SIZE];
static volatile uint16_t s_track_rx_len   = 0;
static volatile bool     s_track_rx_ready = false;
static volatile uint16_t s_track_rx_count = 0; // 累计接收字节数（用于掉线检测）

void BSP_Track_ProcessPacket(uint8_t *data, uint16_t len) {
    for (uint16_t i = 0; i < len; i++) {
        uint8_t c = data[i];
        s_track_rx_count++; // 只要硬件有数据进来就无脑累加

        if (c == '$') { /* 重新同步帧头 */
            s_track_rx_len = 0;
            s_track_rx_ready = false;
        }
        if (!s_track_rx_ready && s_track_rx_len < TRACK_RX_BUF_SIZE) {
            s_track_rx_buf[s_track_rx_len++] = c;
            if (c == '#' || c == '\n') { /* 支持 # 或换行符作为一帧的结束 */
                s_track_rx_ready = true;
            }
        }
    }
}

static void System_Parse_Track(void) {
    if (!s_track_rx_ready) return;

    uint8_t local_buf[TRACK_RX_BUF_SIZE];
    uint16_t local_len = s_track_rx_len;
    
    for (uint16_t i = 0; i < local_len; i++) {
        local_buf[i] = s_track_rx_buf[i];
    }
    
    s_track_rx_len = 0;
    s_track_rx_ready = false;

    Track_ParsePacket(&g_track, local_buf, local_len);
}

/************************** 顶层初始化与调度 (完全基于OOP与Broker) **************************/

void Schedule_Init(void)
{
    /* 底层初始化树 */
    BSP_System_Init();
    BSP_UART_Init();
    BSP_Timer_Init();
    BSP_Timer_ScheduleInit();
    BSP_PWM_Init();

    /* 1. 总线分配中心发车 */
    Broker_Init();
    Track_Init(&g_track, track_send_adapter);
    JY61P_Init(&g_jy61p);
    INS_Init(&g_ins);
    VOFA_RegisterSend(bt_send_adapter);
    
    /* 2. 组装化实例化外设对象 注入真正的硬件驱动与配置 */
    MotorConfig_t mc_l = { .max_duty = 1000, .dead_zone = 5, .hw_api = { .SetDir = _hw_left_set_dir, .SetPWM = _hw_left_set_pwm } };
    MotorConfig_t mc_r = { .max_duty = 1000, .dead_zone = 5, .hw_api = { .SetDir = _hw_right_set_dir, .SetPWM = _hw_right_set_pwm } };
    Motor_Init(&g_motor_left,  &mc_l);
    Motor_Init(&g_motor_right, &mc_r);

    /* 启动马达驱动唤醒 */
    BSP_TB6612_SetEnable(true);
    BSP_System_EnableIRQ();

    /* Mode 状态机初始化 */
    Mode_Init();

    /* 编码器初始化 */
    BSP_Encoder_Init();

    MotorEncoder_Init_Config_t enc_cfg_l = {
        .GetPulseDiff      = BSP_Encoder_GetLeftDiff,
        .TotalResolution   = TOTAL_RESOLUTION,
        .WheelCircumference = WHEEL_CIRCUMFERENCE
    };
    MotorEncoder_Init_Config_t enc_cfg_r = {
        .GetPulseDiff      = BSP_Encoder_GetRightDiff,
        .TotalResolution   = TOTAL_RESOLUTION,
        .WheelCircumference = WHEEL_CIRCUMFERENCE
    };
    MotorEncoder_Init(&g_enc_left,  &enc_cfg_l);
    MotorEncoder_Init(&g_enc_right, &enc_cfg_r);

    /* 启动 QEI 采样定时器 */
    BSP_Timer_QEI_Start();

    /* 速度闭环 PID 初始化 */
    PID_Init_Config_s spd_cfg = {
        .Kp            = PID_WHEEL_KP,
        .Ki            = PID_WHEEL_KI,
        .Kd            = PID_WHEEL_KD,
        .MaxOut        = PID_WHEEL_MAX_OUT,
        .IntegralLimit = PID_WHEEL_INTEGRAL_LIMIT,
        .DeadBand      = PID_WHEEL_DEADBAND,
        .Improve       = PID_Integral_Limit
    };
    PIDInit(&s_pid_speed_left,  &spd_cfg);
    PIDInit(&s_pid_speed_right, &spd_cfg);

    static const char msg[] = "\r\n[SYSTEM BOOT] MSMP0G3507 V2.0 (Right Motor HW Inverted!)\r\n";
    BSP_BT_SendPacket((uint8_t *)msg, sizeof(msg) - 1);
}

void Schedule_Run(void) { SampleAppTaskScheduler(); }

void SampleAppTask1ms(uint16_t wNumTicks)
{
    (void)wNumTicks;

    /* 5ms 快速控制环：提升红外响应+电机刷新频率（200Hz） */
    static uint8_t s_5ms_cnt = 0;
    s_5ms_cnt++;
    if (s_5ms_cnt >= 5) {
        s_5ms_cnt = 0;

        /* 抽取循迹数据（如果有新帧） */
        System_Parse_Track();

        /* 仅在有新数据时更新循迹误差 */
        if (g_track.data_ready) {
            float track_err = Track_GetError(&g_track);
            Broker_Publish_Float(TOPIC_TRACK_ERROR, track_err);
            g_track.data_ready = 0;  /* 消费标记，防止旧数据反复喂 PID */
        }

        /* 所有运动模式都走 5ms 快速通道 + 斜坡 + 速度 PID 闭环 */
        CarMode_e cur = Mode_GetMode();
        static CarMode_e s_last_mode = CAR_MODE_IDLE;
        
        if (cur != s_last_mode) {
            /* 模式切换瞬间，强制清零速度PID，防止历史积分导致起步突变 */
            PID_Clear(&s_pid_speed_left);
            PID_Clear(&s_pid_speed_right);
            s_last_mode = cur;
        }

        if (cur != CAR_MODE_IDLE && cur != CAR_MODE_DONE) {
            Mode_Exec();

            /* Mode 输出目标轮速 (mm/s) → 斜坡限速 → 速度 PID → PWM */
            float tgt_l = Broker_Subscribe_Float(TOPIC_TARGET_SPEED_LEFT);
            float tgt_r = Broker_Subscribe_Float(TOPIC_TARGET_SPEED_RIGHT);

            /* 斜坡：|dv| ≤ SQUARE_RAMP_ACCEL × 0.005s 每周期 */
            const float max_delta = SQUARE_RAMP_ACCEL * 0.005f;
            s_ramp_left  = ramp_step(s_ramp_left,  tgt_l, max_delta);
            s_ramp_right = ramp_step(s_ramp_right, tgt_r, max_delta);

            float spd_l = Broker_Subscribe_Float(TOPIC_SPEED_LEFT);
            float spd_r = Broker_Subscribe_Float(TOPIC_SPEED_RIGHT);
            float pid_l = PIDCalculate(&s_pid_speed_left,  spd_l, s_ramp_left,  0.005f);
            float pid_r = PIDCalculate(&s_pid_speed_right, spd_r, s_ramp_right, 0.005f);
            float pwm_l = s_ramp_left  * PID_WHEEL_FF + pid_l;
            float pwm_r = s_ramp_right * PID_WHEEL_FF + pid_r;
            Motor_SetDrive(&g_motor_left,  pwm_l);
            Motor_SetDrive(&g_motor_right, pwm_r);
        } else {
            /* IDLE/DONE：把斜坡状态衰减归零，下次进入运动从 0 起步 */
            const float max_delta = SQUARE_RAMP_ACCEL * 0.005f;
            s_ramp_left  = ramp_step(s_ramp_left,  0.0f, max_delta);
            s_ramp_right = ramp_step(s_ramp_right, 0.0f, max_delta);
            
            /* 强制切断动力输出（除非用户使用 S 指令切入了底层闭环调试模式） */
            if (s_speed_ctrl_mode == SPEED_CTRL_OPEN) {
                Motor_SetDrive(&g_motor_left,  0.0f);
                Motor_SetDrive(&g_motor_right, 0.0f);
            }
        }
    }
}

void SampleAppTask10ms(uint16_t wNumTicks)
{
    (void)wNumTicks;

    /* Step A: 蓝牙指令解析 (仅 10ms 通道处理) */
    System_Parse_BT();
    
    // 1. IMU 底层解析（10ms 足够，JY61P 输出 ~100Hz）
    JY61P_Parse_Task(&g_jy61p);
    
    // 2. 角度偏置初始化（仅在首帧角度数据完整解析后才触发）
    if (!g_yaw_calibrated && g_jy61p.data_ready) {
        g_yaw_offset = g_jy61p.yaw;
        g_yaw_calibrated = true;
    }
    g_yaw_current = g_jy61p.yaw - g_yaw_offset;
    
    // 3. 把核心传感原始数据发布到总线 (向应用层暴露)
    Broker_Publish_Float(TOPIC_JY61P_YAW, g_yaw_current);
    Broker_Publish_Float(TOPIC_JY61P_AX, g_jy61p.ax);
    Broker_Publish_Float(TOPIC_JY61P_WZ, g_jy61p.wz);  /* 串级 PID 内环反馈 */

    Broker_Publish_Float(TOPIC_INS_SPEED_X, 0.0f);
    Broker_Publish_Float(TOPIC_INS_MILEAGE_X, 0.0f);

    /* Step B: 编码器测速更新 (10ms = 0.01s) */
    MotorEncoder_Update(&g_enc_left,  0.01f);
    MotorEncoder_Update(&g_enc_right, 0.01f);
    Broker_Publish_Float(TOPIC_SPEED_LEFT,  g_enc_left.WheelSpeed);
    Broker_Publish_Float(TOPIC_SPEED_RIGHT, g_enc_right.WheelSpeed);
    Broker_Publish_Float(TOPIC_DISTANCE_LEFT,  (float)g_enc_left.TotalDist);
    Broker_Publish_Float(TOPIC_DISTANCE_RIGHT, (float)g_enc_right.TotalDist);

    /* Step C: 仅 IDLE 模式在 10ms 通道执行 Mode + 电机 */
    /*         其它运动模式已在 5ms 快速通道处理，此处不重复 */
    CarMode_e cur = Mode_GetMode();
    if (cur == CAR_MODE_IDLE) {
        if (s_speed_ctrl_mode == SPEED_CTRL_CLOSED) {
            /* 速度闭环：前馈 + PID 修正 (target=0 时 PID 自动反向制动) */
            float tgt_l = Broker_Subscribe_Float(TOPIC_TARGET_WHEEL_SPEED_LEFT);
            float tgt_r = Broker_Subscribe_Float(TOPIC_TARGET_WHEEL_SPEED_RIGHT);
            float pid_l = PIDCalculate(&s_pid_speed_left,  g_enc_left.WheelSpeed,  tgt_l, 0.01f);
            float pid_r = PIDCalculate(&s_pid_speed_right, g_enc_right.WheelSpeed, tgt_r, 0.01f);
            float pwm_l = tgt_l * PID_WHEEL_FF + pid_l;
            float pwm_r = tgt_r * PID_WHEEL_FF + pid_r;
            Motor_SetDrive(&g_motor_left,  pwm_l);
            Motor_SetDrive(&g_motor_right, pwm_r);
        } else {
            /* 开环：直接 PWM */
            Mode_Exec();
            float target_l = Broker_Subscribe_Float(TOPIC_TARGET_SPEED_LEFT);
            float target_r = Broker_Subscribe_Float(TOPIC_TARGET_SPEED_RIGHT);
            Motor_SetDrive(&g_motor_left, target_l);
            Motor_SetDrive(&g_motor_right, target_r);
        }
    }
}

void SampleAppTask100ms(void)
{
    float pl = Broker_Subscribe_Float(TOPIC_TARGET_SPEED_LEFT);
    float pr = Broker_Subscribe_Float(TOPIC_TARGET_SPEED_RIGHT);
    float err = Broker_Subscribe_Float(TOPIC_TRACK_ERROR);
    int32_t mode = Broker_Subscribe_Int(TOPIC_CAR_MODE);
    int32_t task = Broker_Subscribe_Int(TOPIC_TASK_ID);
    float sl = Broker_Subscribe_Float(TOPIC_SPEED_LEFT);
    float sr = Broker_Subscribe_Float(TOPIC_SPEED_RIGHT);
    
    static uint32_t s_time_ms = 0;
    s_time_ms += 100;

    float dl = Broker_Subscribe_Float(TOPIC_DISTANCE_LEFT);
    float dr = Broker_Subscribe_Float(TOPIC_DISTANCE_RIGHT);
    float dist = (dl + dr) * 0.5f;

    float wz = Broker_Subscribe_Float(TOPIC_JY61P_WZ);

    static char tx_buf[260];
    int len;
    float current_speed = (sl + sr) * 0.5f;
    
    if (s_speed_ctrl_mode == SPEED_CTRL_CLOSED) {
        float tl = Broker_Subscribe_Float(TOPIC_TARGET_WHEEL_SPEED_LEFT);
        float tr = Broker_Subscribe_Float(TOPIC_TARGET_WHEEL_SPEED_RIGHT);
        len = snprintf(tx_buf, sizeof(tx_buf),
            "[%lu]M%d IR:%u,%u,%u,%u,%u,%u,%u,%u Y:%.1f V:%.0f Wz:%.1f D:%.0f TL:%.0f TR:%.0f SL:%.0f SR:%.0f\r\n",
            (unsigned long)s_time_ms, (int)mode,
            g_track.adc_value[0], g_track.adc_value[1], g_track.adc_value[2], g_track.adc_value[3],
            g_track.adc_value[4], g_track.adc_value[5], g_track.adc_value[6], g_track.adc_value[7],
            g_yaw_current, current_speed, wz, dist, tl, tr, sl, sr);
    } else {
        len = snprintf(tx_buf, sizeof(tx_buf),
            "[%lu]M%d IR:%u,%u,%u,%u,%u,%u,%u,%u Y:%.1f V:%.0f Wz:%.1f D:%.0f TL:%.0f TR:%.0f SL:%.0f SR:%.0f\r\n",
            (unsigned long)s_time_ms, (int)mode,
            g_track.adc_value[0], g_track.adc_value[1], g_track.adc_value[2], g_track.adc_value[3],
            g_track.adc_value[4], g_track.adc_value[5], g_track.adc_value[6], g_track.adc_value[7],
            g_yaw_current, current_speed, wz, dist, pl, pr, sl, sr);
    }
    if (len > 0) {
        BSP_BT_SendPacket((uint8_t *)tx_buf, (uint16_t)len);
    }

    /* ==================================================
     * 独立状态机：S2 按键触发 + LED 闪烁 + 发车
     * 运行于 100ms 任务槽，不阻塞任何底层代码
     * ================================================== */
    static int ui_state = 0;       // 0:等待按键, 1:闪烁中
    static int blink_count = 0;    // 剩余闪烁次数 (半周期)

    if (ui_state == 0) {
        // 假设按键按下为低电平
        if (DL_GPIO_readPins(GPIO_UI_KEY_S2_PORT, GPIO_UI_KEY_S2_PIN) == 0) {
            ui_state = 1;
            blink_count = 6;  // 闪烁3次 (亮-灭-亮-灭-亮-灭 = 6个半周期)
            
            // 确保发车前车是停的
            Mode_SetIdle(); 
        }
    } 
    else if (ui_state == 1) {
        if (blink_count > 0) {
            // 每 100ms 翻转一次 LED 状态
            DL_GPIO_togglePins(GPIO_UI_LED_PORT, GPIO_UI_LED_PIN);
            blink_count--;
        } else {
            // 闪烁结束，确保 LED 熄灭
            DL_GPIO_clearPins(GPIO_UI_LED_PORT, GPIO_UI_LED_PIN);
            
            // 发发发车！
            Mode_SetTask(1); 
            
            ui_state = 0; // 重置状态，允许下次再按
        }
    }
}

void SampleAppTask1sec(void) {
    static uint16_t last_track_rx_count = 0;

    /* 防掉线/防异步启动：如果过去1秒钟，底层追踪 UART 完全没有收到任何新数据 */
    if (s_track_rx_count == last_track_rx_count) {
        /* 说明传感器死机了，或者开机时单片机跑太快把启动指令发飞了，这里重新补发启动指令 */
        static const uint8_t cmd[] = "$0,1,0#";
        track_send_adapter((uint8_t *)cmd, sizeof(cmd) - 1);
    }
    last_track_rx_count = s_track_rx_count;
}
void SampleAppTask1min(void) {}
