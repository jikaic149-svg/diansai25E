/**
 * @file HT04.c
 * @brief 航特HT04电机驱动实现（MIT协议，CAN抽象）
 */

#include "HT04.h"
#include "../../Application/config/general_def.h"
#include <stdlib.h>
#include <string.h>

static uint8_t idx = 0;
static HTMotorInstance *ht_motor_instance[HT_MOTOR_CNT] = {NULL};
static HT_CANTransmit_t ht_can_transmit = (void *)0;
static HT_GetDeltaT_t   ht_get_delta_t  = (void *)0;

static uint16_t float_to_uint(float x, float x_min, float x_max, uint8_t bits)
{
    float span = x_max - x_min;
    return (uint16_t)((x - x_min) * ((float)((1 << bits) - 1)) / span);
}

static float uint_to_float(int x_int, float x_min, float x_max, int bits)
{
    float span = x_max - x_min;
    return ((float)x_int) * span / ((float)((1 << bits) - 1)) + x_min;
}

/* 简易滑动平均滤波 */
static float AverageFilter(float new_val, float *buff, uint8_t size)
{
    float sum = 0;
    for (uint8_t i = size - 1; i > 0; i--)
    {
        buff[i] = buff[i - 1];
        sum += buff[i];
    }
    buff[0] = new_val;
    sum += new_val;
    return sum / (float)size;
}

void HTMotorRegisterHW(HT_CANTransmit_t can_send, HT_GetDeltaT_t get_dt)
{
    ht_can_transmit = can_send;
    ht_get_delta_t  = get_dt;
}

void HTMotorSetMode(HTMotor_Mode_t cmd, HTMotorInstance *motor)
{
    if (!ht_can_transmit) return;
    uint8_t buf[8];
    memset(buf, 0xff, 7);
    buf[7] = (uint8_t)cmd;
    ht_can_transmit(motor->tx_id, buf, motor->can_bus);
}

void HTMotorDecode(uint32_t rx_id, uint8_t *rx_data, uint8_t can_bus)
{
    for (uint8_t i = 0; i < idx; i++)
    {
        HTMotorInstance *motor = ht_motor_instance[i];
        if (motor->rx_id == rx_id && motor->can_bus == can_bus)
        {
            HTMotor_Measure_t *measure = &motor->measure;
            uint16_t tmp;

            if (ht_get_delta_t)
                measure->feed_dt = ht_get_delta_t(&measure->feed_cnt);

            measure->last_angle = measure->total_angle;
            tmp = (uint16_t)((rx_data[1] << 8) | rx_data[2]);
            measure->total_angle = uint_to_float(tmp, P_MIN, P_MAX, 16);

            tmp = (uint16_t)((rx_data[3] << 4) | (rx_data[4] >> 4));
            measure->speed_rads = AverageFilter(
                uint_to_float(tmp, V_MIN, V_MAX, 12) - HT_SPEED_BIAS,
                measure->speed_buff, SPEED_BUFFER_SIZE);

            tmp = (uint16_t)(((rx_data[4] & 0x0f) << 8) | rx_data[5]);
            measure->real_current = 0.9f * uint_to_float(tmp, T_MIN, T_MAX, 12) +
                                    0.1f * measure->real_current;
            return;
        }
    }
}

HTMotorInstance *HTMotorInit(Motor_Init_Config_s *config)
{
    HTMotorInstance *motor = (HTMotorInstance *)malloc(sizeof(HTMotorInstance));
    memset(motor, 0, sizeof(HTMotorInstance));

    motor->motor_settings = config->controller_setting_init_config;
    PIDInit(&motor->current_PID, &config->controller_param_init_config.current_PID);
    PIDInit(&motor->speed_PID, &config->controller_param_init_config.speed_PID);
    PIDInit(&motor->angle_PID, &config->controller_param_init_config.angle_PID);
    motor->other_angle_feedback_ptr = config->controller_param_init_config.other_angle_feedback_ptr;
    motor->other_speed_feedback_ptr = config->controller_param_init_config.other_speed_feedback_ptr;

    motor->tx_id   = config->can_init_config.tx_id;
    motor->rx_id   = config->can_init_config.rx_id;
    motor->can_bus = config->can_init_config.can_bus;

    motor->stop_flag = MOTOR_ENABLED;
    ht_motor_instance[idx++] = motor;

    HTMotorSetMode(CMD_MOTOR_MODE, motor);
    return motor;
}

void HTMotorSetRef(HTMotorInstance *motor, float ref) { motor->pid_ref = ref; }
void HTMotorStop(HTMotorInstance *motor) { motor->stop_flag = MOTOR_STOP; }
void HTMotorEnable(HTMotorInstance *motor) { motor->stop_flag = MOTOR_ENABLED; }
void HTMotorOuterLoop(HTMotorInstance *motor, Closeloop_Type_e type) { motor->motor_settings.outer_loop_type = type; }

void HTMotorControl(void)
{
    if (!ht_can_transmit) return;

    for (uint8_t i = 0; i < idx; i++)
    {
        HTMotorInstance *motor = ht_motor_instance[i];
        HTMotor_Measure_t *measure = &motor->measure;
        Motor_Control_Setting_s *setting = &motor->motor_settings;
        float pid_ref = motor->pid_ref;
        float pid_measure, set;

        // 位置环
        if ((setting->close_loop_type & ANGLE_LOOP) && setting->outer_loop_type == ANGLE_LOOP)
        {
            pid_measure = (setting->angle_feedback_source == OTHER_FEED)
                ? *motor->other_angle_feedback_ptr : measure->total_angle;
            pid_ref = PIDCalculate(&motor->angle_PID, pid_measure * RAD_2_DEGREE, pid_ref, 0.001f);
        }

        // 速度环
        if ((setting->close_loop_type & SPEED_LOOP) && (setting->outer_loop_type & (ANGLE_LOOP | SPEED_LOOP)))
        {
            if (setting->feedforward_flag & SPEED_FEEDFORWARD)
                pid_ref += *motor->speed_feedforward_ptr;
            pid_measure = (setting->angle_feedback_source == OTHER_FEED)
                ? *motor->other_speed_feedback_ptr : measure->speed_rads;
            pid_ref = PIDCalculate(&motor->speed_PID, pid_measure * RAD_2_DEGREE, pid_ref, 0.001f);
        }

        // 电流环
        if (setting->feedforward_flag & CURRENT_FEEDFORWARD)
            pid_ref += *motor->current_feedforward_ptr;
        if (setting->close_loop_type & CURRENT_LOOP)
            pid_ref = PIDCalculate(&motor->current_PID, measure->real_current, pid_ref, 0.001f);

        set = pid_ref;
        if (setting->motor_reverse_flag == MOTOR_DIRECTION_REVERSE)
            set *= -1;

        LIMIT_MIN_MAX(set, T_MIN, T_MAX);
        uint16_t tmp = float_to_uint(set, T_MIN, T_MAX, 12);
        if (motor->stop_flag == MOTOR_STOP)
            tmp = float_to_uint(0, T_MIN, T_MAX, 12);

        // 构造MIT协议帧(仅更新力矩位)
        uint8_t buf[8] = {0};
        buf[6] = (uint8_t)(tmp >> 8);
        buf[7] = (uint8_t)(tmp & 0xff);

        ht_can_transmit(motor->tx_id, buf, motor->can_bus);
    }
}
