/**
 * @file LK9025.c
 * @brief 瓴控LK9025电机驱动实现（多电机CAN指令，CAN抽象）
 */

#include "LK9025.h"
#include "../../Application/config/general_def.h"
#include <stdlib.h>
#include <string.h>

#define SPEED_SMOOTH_COEF_LK  0.85f
#define CURRENT_SMOOTH_COEF_LK 0.9f

static uint8_t idx = 0;
static LKMotorInstance *lkmotor_instance[LK_MOTOR_MX_CNT] = {NULL};
static LK_CANTransmit_t lk_can_transmit = (void *)0;
static LK_GetDeltaT_t   lk_get_delta_t  = (void *)0;

/* 多电机发送缓冲区 */
static uint8_t  lk_tx_buff[8] = {0};
static uint32_t lk_multi_tx_id = 0x280;
static uint8_t  lk_can_bus_first = 0;

void LKMotorRegisterHW(LK_CANTransmit_t can_send, LK_GetDeltaT_t get_dt)
{
    lk_can_transmit = can_send;
    lk_get_delta_t  = get_dt;
}

void LKMotorDecode(uint32_t rx_id, uint8_t *rx_data, uint8_t can_bus)
{
    for (uint8_t i = 0; i < idx; i++)
    {
        LKMotorInstance *motor = lkmotor_instance[i];
        if (motor->rx_id == rx_id && motor->can_bus == can_bus)
        {
            LKMotor_Measure_t *measure = &motor->measure;

            if (lk_get_delta_t)
                measure->feed_dt = lk_get_delta_t(&measure->feed_dwt_cnt);

            measure->last_ecd = measure->ecd;
            measure->ecd = (uint16_t)((rx_data[7] << 8) | rx_data[6]);
            measure->angle_single_round = ECD_ANGLE_COEF_LK * measure->ecd;

            measure->speed_rads = (1 - SPEED_SMOOTH_COEF_LK) * measure->speed_rads +
                                  DEGREE_2_RAD * SPEED_SMOOTH_COEF_LK * (float)((int16_t)(rx_data[5] << 8 | rx_data[4]));

            measure->real_current = (1 - CURRENT_SMOOTH_COEF_LK) * measure->real_current +
                                    CURRENT_SMOOTH_COEF_LK * (float)((int16_t)(rx_data[3] << 8 | rx_data[2]));

            measure->temperature = rx_data[1];

            if (measure->ecd - measure->last_ecd > 32768)
                measure->total_round--;
            else if (measure->ecd - measure->last_ecd < -32768)
                measure->total_round++;
            measure->total_angle = measure->total_round * 360 + measure->angle_single_round;
            return;
        }
    }
}

LKMotorInstance *LKMotorInit(Motor_Init_Config_s *config)
{
    LKMotorInstance *motor = (LKMotorInstance *)malloc(sizeof(LKMotorInstance));
    memset(motor, 0, sizeof(LKMotorInstance));

    motor->motor_settings = config->controller_setting_init_config;
    PIDInit(&motor->current_PID, &config->controller_param_init_config.current_PID);
    PIDInit(&motor->speed_PID, &config->controller_param_init_config.speed_PID);
    PIDInit(&motor->angle_PID, &config->controller_param_init_config.angle_PID);
    motor->other_angle_feedback_ptr = config->controller_param_init_config.other_angle_feedback_ptr;
    motor->other_speed_feedback_ptr = config->controller_param_init_config.other_speed_feedback_ptr;

    // LK电机rx_id = 0x140 + motor_id, tx_id保存原始motor_id用于计算偏移
    motor->tx_id     = config->can_init_config.tx_id;
    motor->rx_id     = 0x140 + config->can_init_config.tx_id;
    motor->can_bus   = config->can_init_config.can_bus;
    motor->motor_idx = idx;

    if (idx == 0)
        lk_can_bus_first = motor->can_bus;

    motor->stop_flag = MOTOR_ENABLED;
    lkmotor_instance[idx++] = motor;
    return motor;
}

void LKMotorSetRef(LKMotorInstance *motor, float ref) { motor->pid_ref = ref; }
void LKMotorStop(LKMotorInstance *motor) { motor->stop_flag = MOTOR_STOP; }
void LKMotorEnable(LKMotorInstance *motor) { motor->stop_flag = MOTOR_ENABLED; }

void LKMotorControl(void)
{
    if (!lk_can_transmit || !idx) return;

    float pid_measure, pid_ref;
    int16_t set;

    for (uint8_t i = 0; i < idx; ++i)
    {
        LKMotorInstance *motor = lkmotor_instance[i];
        LKMotor_Measure_t *measure = &motor->measure;
        Motor_Control_Setting_s *setting = &motor->motor_settings;
        pid_ref = motor->pid_ref;

        if ((setting->close_loop_type & ANGLE_LOOP) && setting->outer_loop_type == ANGLE_LOOP)
        {
            pid_measure = (setting->angle_feedback_source == OTHER_FEED)
                ? *motor->other_angle_feedback_ptr : measure->real_current;
            pid_ref = PIDCalculate(&motor->angle_PID, pid_measure, pid_ref, 0.001f);
            if (setting->feedforward_flag & SPEED_FEEDFORWARD)
                pid_ref += *motor->speed_feedforward_ptr;
        }

        if ((setting->close_loop_type & SPEED_LOOP) && (setting->outer_loop_type & (ANGLE_LOOP | SPEED_LOOP)))
        {
            pid_measure = (setting->angle_feedback_source == OTHER_FEED)
                ? *motor->other_speed_feedback_ptr : measure->speed_rads;
            pid_ref = PIDCalculate(&motor->angle_PID, pid_measure, pid_ref, 0.001f);
            if (setting->feedforward_flag & CURRENT_FEEDFORWARD)
                pid_ref += *motor->current_feedforward_ptr;
        }

        if (setting->close_loop_type & CURRENT_LOOP)
            pid_ref = PIDCalculate(&motor->current_PID, measure->real_current, pid_ref, 0.001f);

        set = (int16_t)pid_ref;
        if (setting->motor_reverse_flag == MOTOR_DIRECTION_REVERSE)
            set *= -1;

        // 多电机命令: motor_idx * 2 作为在发送buff中的偏移
        memcpy(lk_tx_buff + motor->motor_idx * 2, &set, sizeof(int16_t));

        if (motor->stop_flag == MOTOR_STOP)
            memset(lk_tx_buff + motor->motor_idx * 2, 0, sizeof(int16_t));
    }

    lk_can_transmit(lk_multi_tx_id, lk_tx_buff, lk_can_bus_first);
}
