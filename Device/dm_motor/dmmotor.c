/**
 * @file dmmotor.c
 * @brief 达妙(DM)电机驱动实现（MIT协议，CAN抽象）
 */

#include "dmmotor.h"
#include "../../Application/config/general_def.h"
#include <stdlib.h>
#include <string.h>

static uint8_t idx = 0;
static DMMotorInstance *dm_motor_instance[DM_MOTOR_CNT] = {NULL};
static DM_CANTransmit_t dm_can_transmit = (void *)0;

/* float ↔ uint 映射 */
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

void DMMotorRegisterHW(DM_CANTransmit_t can_send)
{
    dm_can_transmit = can_send;
}

void DMMotorSetMode(DMMotor_Mode_e cmd, DMMotorInstance *motor)
{
    if (!dm_can_transmit) return;
    uint8_t buf[8];
    memset(buf, 0xff, 7);
    buf[7] = (uint8_t)cmd;
    dm_can_transmit(motor->tx_id, buf, motor->can_bus);
}

void DMMotorDecode(uint32_t rx_id, uint8_t *rx_data, uint8_t can_bus)
{
    for (uint8_t i = 0; i < idx; i++)
    {
        DMMotorInstance *motor = dm_motor_instance[i];
        if (motor->rx_id == rx_id && motor->can_bus == can_bus)
        {
            DM_Motor_Measure_s *measure = &motor->measure;
            uint16_t tmp;

            measure->id    = rx_data[0] & 0x0F;
            measure->state = (rx_data[0] >> 4) & 0x0F;

            measure->last_position = measure->position;
            tmp = (uint16_t)((rx_data[1] << 8) | rx_data[2]);
            measure->position = uint_to_float(tmp, DM_P_MIN, DM_P_MAX, 16);

            tmp = (uint16_t)((rx_data[3] << 4) | (rx_data[4] >> 4));
            measure->velocity = uint_to_float(tmp, DM_V_MIN, DM_V_MAX, 12);

            tmp = (uint16_t)(((rx_data[4] & 0x0f) << 8) | rx_data[5]);
            measure->torque = uint_to_float(tmp, DM_T_MIN, DM_T_MAX, 12);

            measure->T_Mos   = (float)rx_data[6];
            measure->T_Rotor = (float)rx_data[7];

            // 多圈计算
            if (measure->position - measure->last_position > 12.5f)
                measure->total_round--;
            else if (measure->position - measure->last_position < -12.5f)
                measure->total_round++;
            measure->total_angle = (measure->position + measure->total_round * 25.0f) * RAD_2_DEGREE;
            measure->velocity *= RAD_2_DEGREE;
            return;
        }
    }
}

DMMotorInstance *DMMotorInit(Motor_Init_Config_s *config)
{
    DMMotorInstance *motor = (DMMotorInstance *)malloc(sizeof(DMMotorInstance));
    memset(motor, 0, sizeof(DMMotorInstance));

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
    dm_motor_instance[idx++] = motor;

    DMMotorSetMode(DM_CMD_MOTOR_MODE, motor);
    return motor;
}

void DMMotorSetRef(DMMotorInstance *motor, float ref) { motor->pid_ref = ref; }
void DMMotorEnable(DMMotorInstance *motor) { motor->stop_flag = MOTOR_ENABLED; }
void DMMotorStop(DMMotorInstance *motor) { motor->stop_flag = MOTOR_STOP; }
void DMMotorOuterLoop(DMMotorInstance *motor, Closeloop_Type_e type) { motor->motor_settings.outer_loop_type = type; }

void DMMotorControl(void)
{
    if (!dm_can_transmit) return;

    for (uint8_t i = 0; i < idx; i++)
    {
        DMMotorInstance *motor = dm_motor_instance[i];
        DM_Motor_Measure_s *measure = &motor->measure;
        Motor_Control_Setting_s *setting = &motor->motor_settings;
        float pid_ref = motor->pid_ref;
        float pid_measure;

        if (setting->motor_reverse_flag == MOTOR_DIRECTION_REVERSE)
            pid_ref *= -1;

        // 位置环
        if ((setting->close_loop_type & ANGLE_LOOP) && setting->outer_loop_type == ANGLE_LOOP)
        {
            pid_measure = (setting->angle_feedback_source == OTHER_FEED)
                ? *motor->other_angle_feedback_ptr : measure->total_angle;
            pid_ref = PIDCalculate(&motor->angle_PID, pid_measure, pid_ref, 0.001f);
        }

        // 速度环
        if ((setting->close_loop_type & SPEED_LOOP) && (setting->outer_loop_type & (ANGLE_LOOP | SPEED_LOOP)))
        {
            if (setting->feedforward_flag & SPEED_FEEDFORWARD)
                pid_ref += *motor->speed_feedforward_ptr;
            pid_measure = (setting->speed_feedback_source == OTHER_FEED)
                ? *motor->other_speed_feedback_ptr : measure->velocity;
            pid_ref = PIDCalculate(&motor->speed_PID, pid_measure, pid_ref, 0.001f);
        }

        if (setting->feedback_reverse_flag == FEEDBACK_DIRECTION_REVERSE)
            pid_ref *= -1;

        float set = pid_ref;
        LIMIT_MIN_MAX(set, DM_T_MIN, DM_T_MAX);

        DMMotor_Send_s send;
        send.position_des = float_to_uint(0, DM_P_MIN, DM_P_MAX, 16);
        send.velocity_des = float_to_uint(0, DM_V_MIN, DM_V_MAX, 12);
        send.torque_des   = float_to_uint(set, DM_T_MIN, DM_T_MAX, 12);
        send.Kp = 0;
        send.Kd = 0;

        if (motor->stop_flag == MOTOR_STOP)
            send.torque_des = float_to_uint(0, DM_T_MIN, DM_T_MAX, 12);

        uint8_t buf[8];
        buf[0] = (uint8_t)(send.position_des >> 8);
        buf[1] = (uint8_t)(send.position_des);
        buf[2] = (uint8_t)(send.velocity_des >> 4);
        buf[3] = (uint8_t)(((send.velocity_des & 0xF) << 4) | (send.Kp >> 8));
        buf[4] = (uint8_t)(send.Kp);
        buf[5] = (uint8_t)(send.Kd >> 4);
        buf[6] = (uint8_t)(((send.Kd & 0xF) << 4) | (send.torque_des >> 8));
        buf[7] = (uint8_t)(send.torque_des);

        dm_can_transmit(motor->tx_id, buf, motor->can_bus);
    }
}
