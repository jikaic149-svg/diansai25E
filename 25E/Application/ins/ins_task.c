/**
 * @file    ins_task.c
 * @brief   Application层 — INS 惯性导航任务实现
 * @note    整合 BMI088(Device) + Mahony(Algorithm) + Bluetooth(Device)
 *          参考 QGimbal: SPI DMA + EXTI DRDY 中断驱动 + TaskNotify 同步
 *
 *          运行模式:
 *          1. 初始化阶段: SPI 轮询读写 BMI088 寄存器 + 5s零飘校准
 *          2. 运行阶段:   EXTI DRDY → SPI DMA 读取 → TaskNotify → Mahony
 *
 *          遵循 Rule 2: 此文件不包含任何 MCU HAL 头文件
 *          通过 Device 层 Register 机制注入 BSP 函数
 */

#include "ins_task.h"
#include "main.h"
#include <math.h>

/* Device 层 */
#include "bmi088.h"
#include "bmi088_reg.h"
#include "bluetooth.h"
#include "vision.h"

/* Algorithm 层 */
#include "attitude_algorithm.h"
#include "bsp_can.h"
#include "config.h"
#include "gimbal_task.h"

/* BSP 层 (仅用于 Init 时注入函数指针) */
#include "bsp_spi.h"
#include "bsp_uart.h"
#include "bsp_uart6.h"
#include "bsp_delay.h"

/* FreeRTOS */
#include "FreeRTOS.h"
#include "task.h"

#include <stdio.h>
#include <string.h>

/* ======================== 跨任务暴露的全局变量 ======================== */
float global_gimbal_yaw = 0.0f;
float global_gimbal_pitch = 0.0f;
float global_imu_yaw_speed = 0.0f;
float global_imu_pitch_speed = 0.0f;

/* ======================== SPI DMA 缓冲区 (参考 QGimbal) ======================== */

#define SPI_DMA_GYRO_LEN      8   /* 1(cmd) + 6(data) + 1(dummy) */
#define SPI_DMA_ACCEL_LEN     9   /* 1(cmd) + 1(dummy) + 6(data) + 1 */
#define SPI_DMA_TEMP_LEN      4   /* 1(cmd) + 1(dummy) + 2(data) */

#define BMI088_GYRO_RX_BUF_DATA_OFFSET  1
#define BMI088_ACCEL_RX_BUF_DATA_OFFSET 2

static uint8_t gyro_dma_rx_buf[SPI_DMA_GYRO_LEN];
static uint8_t gyro_dma_tx_buf[SPI_DMA_GYRO_LEN]  = {0x82, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

static uint8_t accel_dma_rx_buf[SPI_DMA_ACCEL_LEN];
static uint8_t accel_dma_tx_buf[SPI_DMA_ACCEL_LEN] = {0x92, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

static uint8_t temp_dma_rx_buf[SPI_DMA_TEMP_LEN];
static uint8_t temp_dma_tx_buf[SPI_DMA_TEMP_LEN]   = {0xA2, 0xFF, 0xFF, 0xFF};

/* ======================== DMA 状态标志 (参考 QGimbal) ======================== */

#define IMU_DR_SHFITS       0
#define IMU_SPI_SHFITS      1
#define IMU_UPDATE_SHFITS   2

static volatile uint8_t gyro_update_flag = 0;
static volatile uint8_t accel_update_flag = 0;
static volatile uint8_t accel_temp_update_flag = 0;
static volatile uint8_t imu_start_dma_flag = 0;

/* ======================== 模块实例 ======================== */

static BMI088_Instance_t    g_bmi088;
static Bluetooth_Instance_t g_bluetooth;

/* ======================== Device 实例 ======================== */
static Vision_Instance_t g_vision;

/* INS 任务句柄 (用于 TaskNotify) */
static TaskHandle_t ins_task_handle_local = NULL;

/* IMU 数据 (DMA 解析后) */
static float imu_gyro[3]  = {0};
static float imu_accel[3] = {0};
static float imu_temp     = 0;

/* Yaw 运行时漂移补偿 (参考 QGimbal) - 目前已使用静态补偿 */
// static float yaw_runtime_bias = 0.0f;

/* ======================== 前向声明 ======================== */

static void imu_cmd_spi_dma(void);
static void gyro_read_over(const uint8_t *rx_buf, float gyro_out[3]);
static void accel_read_over(const uint8_t *rx_buf, float accel_out[3]);
static void temperature_read_over(const uint8_t *rx_buf, float *temp_out);

/* ======================== BSP <-> Device 粘合 ======================== */

static bool bt_send_adapter(const uint8_t *data, uint16_t len)
{
    return BSP_UART1_SendBytes(data, len);
}

/* 把 BSP_UART1 收到的字节喂给 Bluetooth Device */
static void bt_byte_dispatcher(uint8_t b)
{
    Bluetooth_FeedRxByte(&g_bluetooth, b);
}

extern float ext_target_yaw;
extern uint8_t flag_ext_yaw;

/* 蓝牙收到一行 (BSP→Bluetooth 装配后回调到这里, ISR 上下文)
 * 只做轻量级字段提取, 不做 snprintf / 阻塞 API */
static void bt_line_callback(const char *line, uint16_t len)
{
    (void)len;

    if (line[0] == 'Y' || line[0] == 'y') {
        float val = 0.0f;
        int sign = 1;
        uint8_t j = 1;

        while (line[j] == ' ') j++;
        if      (line[j] == '-') { sign = -1; j++; }
        else if (line[j] == '+') {             j++; }

        uint8_t has_num = 0;
        while (line[j] >= '0' && line[j] <= '9') {
            val = val * 10.0f + (line[j] - '0');
            has_num = 1;
            j++;
        }
        if (line[j] == '.') {
            j++;
            float frac = 10.0f;
            while (line[j] >= '0' && line[j] <= '9') {
                val += (line[j] - '0') / frac;
                frac *= 10.0f;
                has_num = 1;
                j++;
            }
        }
        if (has_num) {
            ext_target_yaw = val * sign;
            flag_ext_yaw   = 1;
        }
    }
}

static void bt_cmd_callback(uint8_t cmd_id, const uint8_t *payload, uint8_t len)
{
    switch (cmd_id)
    {
    case BT_CMD_REMOTE:
        if (len >= 1 && payload[0] == 0x01)
        {
            BMI088_CalibrateZeroDrift(&g_bmi088);
        }
        break;
    default:
        break;
    }
}

/* ======================== 初始化 ======================== */

void INS_Init(void)
{
    /* ----- 1. BSP 初始化 ----- */
    BSP_SPI_Init();
    BSP_UART1_Init();
    BSP_UART6_Init();

    /* ----- 1.1. Device 实例化, 注入 BSP 接口 -----
     *   App 层把 BSP 的 send/register 函数指针交给 Device 模块, Device 模块
     *   后续不再直接依赖 BSP, 满足 ti.md Rule 2/3 (跨层解耦 + 注册机制) */
    Bluetooth_Init(&g_bluetooth, bt_send_adapter);
    Bluetooth_RegisterRxCallback(&g_bluetooth, bt_cmd_callback);
    Bluetooth_RegisterLineCallback(&g_bluetooth, bt_line_callback);
    BSP_UART1_RegisterRxCallback(bt_byte_dispatcher);

    Vision_Init(&g_vision, BSP_UART6_SendBytes, BSP_UART6_RegisterRxCallback);

    /* ----- 2. BMI088 初始化 (轮询模式，注入 BSP 函数指针) ----- */
    BMI088_Config_t bmi_config = {
        .spi_read_write = BSP_SPI_ReadWriteByte,
        .accel_cs_low   = BSP_SPI_AccelCS_Low,
        .accel_cs_high  = BSP_SPI_AccelCS_High,
        .gyro_cs_low    = BSP_SPI_GyroCS_Low,
        .gyro_cs_high   = BSP_SPI_GyroCS_High,
        .delay_us       = BSP_Delay_Us,
        .delay_ms       = BSP_Delay_Ms,
    };

    while (BMI088_Init(&g_bmi088, &bmi_config) != 0)
    {
        BSP_Delay_Ms(100);
    }

    /* ----- 3. 零飘校准 (已改为静态补偿，跳过阻塞 5 秒) ----- */
    // BMI088_CalibrateZeroDrift(&g_bmi088);
    // g_bmi088.calib_start_tick = xTaskGetTickCount();

    /* ----- 4. Mahony 滤波器初始化 ----- */
    Filter_Init();

    /* ----- 5. 切换到 SPI DMA 模式 (运行时高速读取) ----- */
    BSP_SPI_DMA_Init();
}

/* ======================== INS 主任务 ======================== */

/**
 * @brief  INS FreeRTOS 任务 — 由 DRDY 中断 + SPI DMA 驱动
 * @note   参考 QGimbal InsTask:
 *         DRDY EXTI → SPI DMA 传输 → DMA完成中断 → TaskNotify → 姿态解算
 *         实际频率由 BMI088 DRDY 决定 (~1kHz for gyro)
 *         如 DRDY 未接线，则回退到 vTaskDelayUntil 定时 200Hz 轮询
 */
void INS_Task(void *argument)
{
    (void)argument;

    /* 保存任务句柄用于 ISR 中的 TaskNotify */
    ins_task_handle_local = xTaskGetCurrentTaskHandle();

    /* 启动 DMA 模式 */
    imu_start_dma_flag = 1;

    uint32_t init_counter = 0; // 替换 TickType_t，使用明确的循环计数
    float yaw_offset = 0.0f, pitch_offset = 0.0f, roll_offset = 0.0f;
    uint32_t bt_send_divider = 0;

    for (;;)
    {
        /* 等待 SPI DMA 完成通知 (如有 DRDY 接线)
         * 超时 10ms 作为回退: 即使无 DRDY 中断也能以 100Hz 运行 */
        ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(10));

        static float gyro_bias[3] = {0.000902f, 0.003273f, -0.001428f};
        static float gyro_sum[3] = {0.0f, 0.0f, 0.0f};

        /* 处理 DMA 读取完成的数据 */
        if (gyro_update_flag & (1 << IMU_UPDATE_SHFITS))
        {
            gyro_update_flag &= ~(1 << IMU_UPDATE_SHFITS);
            gyro_read_over(gyro_dma_rx_buf + BMI088_GYRO_RX_BUF_DATA_OFFSET, imu_gyro);

            // 在极静采集态下，累加原始角速度以计算动态零偏
            if (gimbal_state == GIMBAL_STATE_CALIBRATING) {
                gyro_sum[0] += imu_gyro[0];
                gyro_sum[1] += imu_gyro[1];
                gyro_sum[2] += imu_gyro[2];
            }

            /* 扣除零飘 (未采集完时使用旧的静态值，采集完后使用绝对准确的新值) */
            imu_gyro[0] -= gyro_bias[0];
            imu_gyro[1] -= gyro_bias[1];
            imu_gyro[2] -= gyro_bias[2];
        }

        if (accel_update_flag & (1 << IMU_UPDATE_SHFITS))
        {
            accel_update_flag &= ~(1 << IMU_UPDATE_SHFITS);
            accel_read_over(accel_dma_rx_buf + BMI088_ACCEL_RX_BUF_DATA_OFFSET, imu_accel);
        }

        if (accel_temp_update_flag & (1 << IMU_UPDATE_SHFITS))
        {
            accel_temp_update_flag &= ~(1 << IMU_UPDATE_SHFITS);
            temperature_read_over(temp_dma_rx_buf + BMI088_ACCEL_RX_BUF_DATA_OFFSET, &imu_temp);
        }

        /* 针对 BMI088 芯片特性进行坐标轴重映射（极其重要）
         * BMI088 内部的加速度计和陀螺仪的坐标系是不同的！
         * 根据数据手册：Gyro 的 Y、Z 轴方向与 Accel 的 Y、Z 轴刚好相反。
         * 必须将它们统一到一个坐标系下，否则互补滤波会发生严重的轴耦合错乱。*/
        float gx_deg = imu_gyro[0] * Rad2Ang;
        float gy_deg = -imu_gyro[1] * Rad2Ang; // 翻转 Y 轴，对齐 Accel
        float gz_deg = -imu_gyro[2] * Rad2Ang; // 翻转 Z 轴，对齐 Accel

        MahonyAHRSupdateIMU(gx_deg, gy_deg, gz_deg,
                            imu_accel[0], imu_accel[1], imu_accel[2]);

        /* 角度过零点处理（多圈连续角度累计，消除突变） */
        static float last_Yaw_a = 0.0f, last_Pitch_a = 0.0f, last_Roll_a = 0.0f;
        static float total_Yaw_a = 0.0f, total_Pitch_a = 0.0f, total_Roll_a = 0.0f;
        static uint8_t first_angle = 1;

        if (first_angle) {
            last_Yaw_a = Yaw_a; last_Pitch_a = Pitch_a; last_Roll_a = Roll_a;
            total_Yaw_a = Yaw_a; total_Pitch_a = Pitch_a; total_Roll_a = Roll_a;
            first_angle = 0;
        }

        /* 计算差值，消除 +-180 处的跳变现象 */
        float dyaw = Yaw_a - last_Yaw_a;
        if (dyaw > 180.0f) dyaw -= 360.0f;
        else if (dyaw < -180.0f) dyaw += 360.0f;
        total_Yaw_a += dyaw;
        last_Yaw_a = Yaw_a;

        float dpitch = Pitch_a - last_Pitch_a;
        if (dpitch > 180.0f) dpitch -= 360.0f;
        else if (dpitch < -180.0f) dpitch += 360.0f;
        total_Pitch_a += dpitch;
        last_Pitch_a = Pitch_a;

        float droll = Roll_a - last_Roll_a;
        if (droll > 180.0f) droll -= 360.0f;
        else if (droll < -180.0f) droll += 360.0f;
        total_Roll_a += droll;
        last_Roll_a = Roll_a;

        /* ==================== 多态开机逻辑与按键检测 ==================== */
        static uint8_t key_last_state = 1;
        uint8_t key_curr_state = (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET) ? 0 : 1;
        
        // 检测按键下降沿
        if (key_last_state == 1 && key_curr_state == 0) {
            // 如果还在松弛态，单击按键切入编码器硬锁
            if (gimbal_state == GIMBAL_STATE_RELAX) {
                gimbal_state = GIMBAL_STATE_ENC_LOCK;
                init_counter = 0; // 重置校准计数器
                gyro_sum[0] = 0.0f; // 重置零偏累加器
                gyro_sum[1] = 0.0f;
                gyro_sum[2] = 0.0f;
            } else {
                // 如果已经在运行，再次按下作为急停
                emergency_stop = 1;
            }
        }
        key_last_state = key_curr_state;

        /* 开机 2s 零偏采集逻辑 (仅在硬锁态下进行，此时机械绝对静止) */
        if (gimbal_state == GIMBAL_STATE_CALIBRATING)
        {
            init_counter++;
            if (init_counter >= 2000)
            {
                // 计算出这 2000 次采样的平均角速度，作为新的绝对零偏！
                gyro_bias[0] = gyro_sum[0] / 2000.0f;
                gyro_bias[1] = gyro_sum[1] / 2000.0f;
                gyro_bias[2] = gyro_sum[2] / 2000.0f;
                
                // 将当前的累积角度设为零点
                yaw_offset = total_Yaw_a;
                pitch_offset = total_Pitch_a;
                roll_offset = total_Roll_a;
                
                gimbal_state = GIMBAL_STATE_IMU_STAB; // 零偏消除完成，切入完美增稳态
            }
        }
        
        float out_yaw = fmodf(total_Yaw_a - yaw_offset, 360.0f);
        if (out_yaw < 0.0f) out_yaw += 360.0f;  // 确保始终在 [0, 360)
        float out_pitch = total_Pitch_a - pitch_offset;
        float out_roll = total_Roll_a - roll_offset;

        /* ======================== 全局姿态更新与蓝牙输出 ======================== */
        // 编码器角度 (CAN层已转为角度制 0~360°)
        float enc_pitch_deg = pitch_motor.angle;
        
        // 云台最终 Pitch 和 Yaw (IMU + 编码器融合)
        float gimbal_pitch = out_pitch + (enc_pitch_deg - MOTOR_PITCH_MECH_ZERO);
        float gimbal_yaw   = out_yaw;
        
        // 更新全局变量给控制任务使用
        global_gimbal_pitch = gimbal_pitch;
        global_gimbal_yaw   = gimbal_yaw;
        // 更新角速度给未来可能的 D 环使用 (已转换为度/秒)
        global_imu_yaw_speed   = gz_deg; 
        global_imu_pitch_speed = gy_deg;

        bt_send_divider++;
        if (bt_send_divider >= 10)
        {
            bt_send_divider = 0;

            int16_t  vis_dx  = Vision_GetDx(&g_vision);
            int16_t  vis_dy  = Vision_GetDy(&g_vision);
            uint32_t vis_frm = Vision_GetFrameCount(&g_vision);

            static char txt_buf[128];
            if (gimbal_state == GIMBAL_STATE_RELAX) {
                sprintf(txt_buf, "RELAX dx:%+5d dy:%+5d vis_frm:%lu\r\n",
                        vis_dx, vis_dy, vis_frm);
            } else if (gimbal_state == GIMBAL_STATE_CALIBRATING) {
                sprintf(txt_buf, "CALIB %lu Yaw:%.2f dx:%+5d dy:%+5d\r\n",
                        init_counter, gimbal_yaw, vis_dx, vis_dy);
            } else {
                sprintf(txt_buf, "Yaw:%.2f dx:%+5d dy:%+5d vis_frm:%lu\r\n",
                        gimbal_yaw, vis_dx, vis_dy, vis_frm);
            }
            BSP_UART1_SendBytes((const uint8_t *)txt_buf, strlen(txt_buf));
        }

        /* PA0 按键下降沿 → 给视觉发 'T' (追踪命令) */
        {
            static uint8_t key_last = 1;
            uint8_t key_now = (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET) ? 0 : 1;
            if (key_last == 1 && key_now == 0) {
                Vision_Send(&g_vision, (const uint8_t *)"T", 1);
            }
            key_last = key_now;
        }

        /* ===== 指示灯: 急停红, 正常绿 ===== */
        if (emergency_stop) {
            HAL_GPIO_WritePin(GPIOH, GPIO_PIN_12, GPIO_PIN_SET);    /* 红 */
            HAL_GPIO_WritePin(GPIOH, GPIO_PIN_11, GPIO_PIN_RESET);
        } else {
            HAL_GPIO_WritePin(GPIOH, GPIO_PIN_11, GPIO_PIN_SET);    /* 绿 */
            HAL_GPIO_WritePin(GPIOH, GPIO_PIN_12, GPIO_PIN_RESET);
        }

        /* 极端保护：如果由于瞬间并发数据塞爆了 RX_FIFO 导致 CAN 硬件报出 Overrun 错误锁死，
           我们必须强行清除错误并重新打开中断，否则后续永远进不了接收回调！ */
        extern CAN_HandleTypeDef hcan1;
        if (hcan1.ErrorCode != HAL_CAN_ERROR_NONE) {
            hcan1.ErrorCode = HAL_CAN_ERROR_NONE;
            __HAL_CAN_CLEAR_FLAG(&hcan1, CAN_FLAG_FOV0);
            __HAL_CAN_CLEAR_FLAG(&hcan1, CAN_FLAG_FOV1);
            HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);
        }
    }
}

/* ======================== 蓝牙接收任务 ======================== */

void BT_RxTask(void *argument)
{
    (void)argument;

    for (;;)
    {
        /* 蓝牙接收在 ISR 中完成 (IDLE + DMA)
         * 此任务预留用于扩展 */
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

/* ======================== DMA 数据解析 (参考 QGimbal) ======================== */

static void gyro_read_over(const uint8_t *rx_buf, float gyro_out[3])
{
    int16_t raw;
    raw = (int16_t)((rx_buf[1] << 8) | rx_buf[0]);
    gyro_out[0] = raw * BMI088_GYRO_2000_SEN;
    raw = (int16_t)((rx_buf[3] << 8) | rx_buf[2]);
    gyro_out[1] = raw * BMI088_GYRO_2000_SEN;
    raw = (int16_t)((rx_buf[5] << 8) | rx_buf[4]);
    gyro_out[2] = raw * BMI088_GYRO_2000_SEN;
}

static void accel_read_over(const uint8_t *rx_buf, float accel_out[3])
{
    int16_t raw;
    raw = (int16_t)((rx_buf[1] << 8) | rx_buf[0]);
    accel_out[0] = raw * BMI088_ACCEL_3G_SEN;
    raw = (int16_t)((rx_buf[3] << 8) | rx_buf[2]);
    accel_out[1] = raw * BMI088_ACCEL_3G_SEN;
    raw = (int16_t)((rx_buf[5] << 8) | rx_buf[4]);
    accel_out[2] = raw * BMI088_ACCEL_3G_SEN;
}

static void temperature_read_over(const uint8_t *rx_buf, float *temp_out)
{
    int16_t raw = (int16_t)((rx_buf[0] << 3) | (rx_buf[1] >> 5));
    if (raw > 1023) raw -= 2048;
    *temp_out = raw * BMI088_TEMP_FACTOR + BMI088_TEMP_OFFSET;
}

/* ======================== SPI DMA 调度 (参考 QGimbal) ======================== */

/**
 * @brief  根据标志位启动下一次 SPI DMA 传输
 *         优先级: gyro > accel > temp
 */
static void imu_cmd_spi_dma(void)
{
    /* 陀螺仪 DMA */
    if ((gyro_update_flag & (1 << IMU_DR_SHFITS))
        && !(accel_update_flag & (1 << IMU_SPI_SHFITS))
        && !(accel_temp_update_flag & (1 << IMU_SPI_SHFITS)))
    {
        gyro_update_flag &= ~(1 << IMU_DR_SHFITS);
        gyro_update_flag |= (1 << IMU_SPI_SHFITS);

        BSP_SPI_GyroCS_Low();
        BSP_SPI_DMA_Enable((uint32_t)gyro_dma_tx_buf,
                           (uint32_t)gyro_dma_rx_buf,
                           SPI_DMA_GYRO_LEN);
        return;
    }

    /* 加速度计 DMA */
    if ((accel_update_flag & (1 << IMU_DR_SHFITS))
        && !(gyro_update_flag & (1 << IMU_SPI_SHFITS))
        && !(accel_temp_update_flag & (1 << IMU_SPI_SHFITS)))
    {
        accel_update_flag &= ~(1 << IMU_DR_SHFITS);
        accel_update_flag |= (1 << IMU_SPI_SHFITS);

        BSP_SPI_AccelCS_Low();
        BSP_SPI_DMA_Enable((uint32_t)accel_dma_tx_buf,
                           (uint32_t)accel_dma_rx_buf,
                           SPI_DMA_ACCEL_LEN);
        return;
    }

    /* 温度 DMA */
    if ((accel_temp_update_flag & (1 << IMU_DR_SHFITS))
        && !(gyro_update_flag & (1 << IMU_SPI_SHFITS))
        && !(accel_update_flag & (1 << IMU_SPI_SHFITS)))
    {
        accel_temp_update_flag &= ~(1 << IMU_DR_SHFITS);
        accel_temp_update_flag |= (1 << IMU_SPI_SHFITS);

        BSP_SPI_AccelCS_Low();
        BSP_SPI_DMA_Enable((uint32_t)temp_dma_tx_buf,
                           (uint32_t)temp_dma_rx_buf,
                           SPI_DMA_TEMP_LEN);
        return;
    }
}

/* ======================== EXTI DRDY 中断回调 ======================== */

/**
 * @brief  GPIO EXTI 回调 (HAL 框架调用)
 *         PC4 = INT1_ACCEL (DRDY), PC5 = INT1_GYRO (DRDY)
 *         参考 QGimbal: HAL_GPIO_EXTI_Callback
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == INT1_ACCEL_Pin)
    {
        accel_update_flag |= (1 << IMU_DR_SHFITS);
        accel_temp_update_flag |= (1 << IMU_DR_SHFITS);
        if (imu_start_dma_flag)
            imu_cmd_spi_dma();
    }
    else if (GPIO_Pin == INT1_GRYO_Pin)
    {
        gyro_update_flag |= (1 << IMU_DR_SHFITS);
        if (imu_start_dma_flag)
            imu_cmd_spi_dma();
    }
}

/* ======================== SPI DMA 完成回调 ======================== */

/**
 * @brief  SPI1 RX DMA 完成回调 (覆盖 BSP 弱定义)
 *         参考 QGimbal: DMA2_Stream0_IRQHandler 内部逻辑
 */
void BSP_SPI_DMA_RxCpltCallback(void)
{
    /* 陀螺仪读取完成 */
    if (gyro_update_flag & (1 << IMU_SPI_SHFITS))
    {
        gyro_update_flag &= ~(1 << IMU_SPI_SHFITS);
        gyro_update_flag |= (1 << IMU_UPDATE_SHFITS);
        BSP_SPI_GyroCS_High();
    }

    /* 加速度计读取完成 */
    if (accel_update_flag & (1 << IMU_SPI_SHFITS))
    {
        accel_update_flag &= ~(1 << IMU_SPI_SHFITS);
        accel_update_flag |= (1 << IMU_UPDATE_SHFITS);
        BSP_SPI_AccelCS_High();
    }

    /* 温度读取完成 */
    if (accel_temp_update_flag & (1 << IMU_SPI_SHFITS))
    {
        accel_temp_update_flag &= ~(1 << IMU_SPI_SHFITS);
        accel_temp_update_flag |= (1 << IMU_UPDATE_SHFITS);
        BSP_SPI_AccelCS_High();
    }

    /* 继续下一个 DMA 传输 */
    imu_cmd_spi_dma();

    /* 陀螺仪数据就绪时通知 INS 任务 */
    if (gyro_update_flag & (1 << IMU_UPDATE_SHFITS))
    {
        if (ins_task_handle_local != NULL)
        {
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            vTaskNotifyGiveFromISR(ins_task_handle_local, &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
}

/* ======================== 数据获取 ======================== */

float INS_GetYaw(void)       { return Yaw_a; }
float INS_GetPitch(void)     { return Pitch_a; }
float INS_GetRoll(void)      { return Roll_a; }
float INS_GetYawTotal(void)  { return Yaw_TotalAngle; }
uint8_t INS_IsCalibrated(void) { return g_bmi088.calibrated; }
