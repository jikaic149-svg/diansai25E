/**
 * @file dji_motor.c
 * @brief DJI智能电机驱动实现（CAN抽象，支持M3508/M2006/GM6020/GM6020_NEW）
 * @origin 云台 modules/motor/DJImotor/dji_motor.c
 */

#include "dji_motor.h"
#include "../../Application/config/general_def.h"
#include <stdlib.h>
#include <string.h>

static uint8_t idx = 0;
static DJIMotorInstance *dji_motor_instance[DJI_MOTOR_CNT] = {NULL};

/* 硬件回调 */
static DJI_CANTransmit_t dji_can_transmit = (void *)0;
static DJI_GetDeltaT_t   dji_get_delta_t  = (void *)0;

/**
 * @brief 发送缓冲区: 
 *  CAN总线0~2, 每条总线5组:
 *   [bus*5+0] = 0x1FF (GM6020 id1-4 / M3508 id5-8)
 *   [bus*5+1] = 0x200 (M3508/M2006 id1-4)
 *   [bus*5+2] = 0x2FF (GM6020 id5-8)
 *   [bus*5+3] = 0x1FE (GM6020_NEW id1-4)
 *   [bus*5+4] = 0x2FE (GM6020_NEW id5-8)
 */
#define SENDER_GROUP_CNT 15
static uint8_t sender_tx_buff[SENDER_GROUP_CNT][8];
static uint32_t sender_tx_id[SENDER_GROUP_CNT] = {
    0x1ff, 0x200, 0x2ff, /* can_bus 0 */
    0x1ff, 0x200, 0x2ff, /* can_bus 1 */
    0x1ff, 0x200, 0x2ff, /* can_bus 2 */
    0x1fe, 0x2fe,        /* can_bus 0, GM6020_NEW */
    0x1fe, 0x2fe,        /* can_bus 1, GM6020_NEW */
    0x1fe, 0x2fe,        /* can_bus 2, GM6020_NEW */
};
static uint8_t sender_can_bus[SENDER_GROUP_CNT] = {
    0,0,0, 1,1,1, 2,2,2, 0,0, 1,1, 2,2
};
static uint8_t sender_enable_flag[SENDER_GROUP_CNT] = {0};

void DJIMotorRegisterHW(DJI_CANTransmit_t can_send, DJI_GetDeltaT_t get_dt)
{
    dji_can_transmit = can_send;
    dji_get_delta_t  = get_dt;
}

static void MotorSenderGrouping(DJIMotorInstance *motor, CAN_Init_Config_s *config)
{
    uint8_t motor_id = config->tx_id - 1;
    uint8_t motor_send_num;
    uint8_t motor_grouping;
    uint8_t grouping_offset = config->can_bus * 3; // 0,3,6

    switch (motor->motor_type)
    {
    case M2006:
    case M3508:
        if (motor_id < 4) {
            motor_send_num = motor_id;
            motor_grouping = grouping_offset + 1; // 0x200
        } else {
            motor_send_num = motor_id - 4;
            motor_grouping = grouping_offset + 0; // 0x1FF
        }
        config->rx_id = 0x200 + motor_id + 1;
        break;

    case GM6020:
        if (motor_id < 4) {
            motor_send_num = motor_id;
            motor_grouping = grouping_offset + 0; // 0x1FF
        } else {
            motor_send_num = motor_id - 4;
            motor_grouping = grouping_offset + 2; // 0x2FF
        }
        config->rx_id = 0x204 + motor_id + 1;
        break;

    case GM6020_NEW:
    {
        uint8_t new_offset = 9 + config->can_bus * 2;
        if (motor_id < 4) {
            motor_send_num = motor_id;
            motor_grouping = new_offset + 0; // 0x1FE
        } else {
            motor_send_num = motor_id - 4;
            motor_grouping = new_offset + 1; // 0x2FE
        }
        config->rx_id = 0x204 + motor_id + 1;
        break;
    }
    default:
        return;
    }

    sender_enable_flag[motor_grouping] = 1;
    motor->message_num  = motor_send_num;
    motor->sender_group = motor_grouping;
    motor->rx_id   = config->rx_id;
    motor->can_bus = config->can_bus;
}

/* CAN接收解码 - 由BSP层的CAN中断调用 */
void DJIMotorDecode(uint32_t rx_id, uint8_t *rx_data, uint8_t can_bus)
{
    for (uint8_t i = 0; i < idx; i++)
    {
        DJIMotorInstance *motor = dji_motor_instance[i];
        if (motor->rx_id == rx_id && motor->can_bus == can_bus)
        {
            DJI_Motor_Measure_s *measure = &motor->measure;

            if (dji_get_delta_t)
                motor->dt = dji_get_delta_t(&motor->feed_cnt);

            measure->last_ecd = measure->ecd;
            measure->ecd = ((uint16_t)rx_data[0]) << 8 | rx_data[1];
            measure->angle_single_round = ECD_ANGLE_COEF_DJI * (float)measure->ecd;
            measure->speed_aps = (1.0f - SPEED_SMOOTH_COEF) * measure->speed_aps +
                                 RPM_2_ANGLE_PER_SEC * SPEED_SMOOTH_COEF * (float)((int16_t)(rx_data[2] << 8 | rx_data[3]));
            measure->real_current = (1.0f - CURRENT_SMOOTH_COEF) * measure->real_current +
                                    CURRENT_SMOOTH_COEF * (float)((int16_t)(rx_data[4] << 8 | rx_data[5]));
            measure->temperature = rx_data[6];

            if (measure->ecd - measure->last_ecd > 4096)
                measure->total_round--;
            else if (measure->ecd - measure->last_ecd < -4096)
                measure->total_round++;
            measure->total_angle = measure->total_round * 360 + measure->angle_single_round;
            return;
        }
    }
}

DJIMotorInstance *DJIMotorInit(Motor_Init_Config_s *config)
{
    DJIMotorInstance *instance = (DJIMotorInstance *)malloc(sizeof(DJIMotorInstance));
    memset(instance, 0, sizeof(DJIMotorInstance));

    instance->motor_type = config->motor_type;
    instance->motor_settings = config->controller_setting_init_config;

    PIDInit(&instance->motor_controller.current_PID, &config->controller_param_init_config.current_PID);
    PIDInit(&instance->motor_controller.speed_PID, &config->controller_param_init_config.speed_PID);
    PIDInit(&instance->motor_controller.angle_PID, &config->controller_param_init_config.angle_PID);
    instance->motor_controller.other_angle_feedback_ptr = config->controller_param_init_config.other_angle_feedback_ptr;
    instance->motor_controller.other_speed_feedback_ptr = config->controller_param_init_config.other_speed_feedback_ptr;
    instance->motor_controller.current_feedforward_ptr = config->controller_param_init_config.current_feedforward_ptr;
    instance->motor_controller.speed_feedforward_ptr = config->controller_param_init_config.speed_feedforward_ptr;

    MotorSenderGrouping(instance, &config->can_init_config);

    instance->stop_flag = MOTOR_ENABLED;
    dji_motor_instance[idx++] = instance;
    return instance;
}

void DJIMotorChangeFeed(DJIMotorInstance *motor, Closeloop_Type_e loop, Feedback_Source_e type)
{
    if (loop == ANGLE_LOOP)
        motor->motor_settings.angle_feedback_source = type;
    else if (loop == SPEED_LOOP)
        motor->motor_settings.speed_feedback_source = type;
}

void DJIMotorStop(DJIMotorInstance *motor)
{
    motor->motor_controller.speed_PID.Iout = 0;
    motor->motor_controller.angle_PID.Iout = 0;
    motor->motor_controller.current_PID.Iout = 0;
    motor->stop_flag = MOTOR_STOP;
}

void DJIMotorEnable(DJIMotorInstance *motor)
{
    motor->stop_flag = MOTOR_ENABLED;
}

void DJIMotorOuterLoop(DJIMotorInstance *motor, Closeloop_Type_e outer_loop)
{
    motor->motor_settings.outer_loop_type = outer_loop;
}

void DJIMotorSetRef(DJIMotorInstance *motor, float ref)
{
    motor->motor_controller.pid_ref = ref;
}

void DJIMotorSetExternal(DJIMotorInstance *motor, float external)
{
    motor->motor_controller.output_external = external;
}

void DJIMotorControl(void)
{
    if (!dji_can_transmit) return;

    uint8_t group, num;
    int16_t set;
    DJIMotorInstance *motor;
    Motor_Control_Setting_s *motor_setting;
    Motor_Controller_s *motor_controller;
    DJI_Motor_Measure_s *measure;
    float pid_measure, pid_ref, output_external;

    for (uint8_t i = 0; i < idx; ++i)
    {
        motor = dji_motor_instance[i];
        motor_setting = &motor->motor_settings;
        motor_controller = &motor->motor_controller;
        measure = &motor->measure;
        pid_ref = motor_controller->pid_ref;
        output_external = motor_controller->output_external;

        if (motor_setting->motor_reverse_flag == MOTOR_DIRECTION_REVERSE)
            pid_ref *= -1;

        // 位置环
        if ((motor_setting->close_loop_type & ANGLE_LOOP) && motor_setting->outer_loop_type == ANGLE_LOOP)
        {
            if (motor_setting->angle_feedback_source == OTHER_FEED)
                pid_measure = *motor_controller->other_angle_feedback_ptr;
            else
                pid_measure = measure->total_angle;
            pid_ref = PIDCalculate(&motor_controller->angle_PID, pid_measure, pid_ref, 0.001f);
        }

        // 速度环
        if ((motor_setting->close_loop_type & SPEED_LOOP) && (motor_setting->outer_loop_type & (ANGLE_LOOP | SPEED_LOOP)))
        {
            if (motor_setting->feedforward_flag & SPEED_FEEDFORWARD)
                pid_ref += *motor_controller->speed_feedforward_ptr;
            if (motor_setting->speed_feedback_source == OTHER_FEED)
                pid_measure = *motor_controller->other_speed_feedback_ptr;
            else
                pid_measure = measure->speed_aps;
            pid_ref = PIDCalculate(&motor_controller->speed_PID, pid_measure, pid_ref, 0.001f);
        }

        // 电流环
        if (motor_setting->feedforward_flag & CURRENT_FEEDFORWARD)
            pid_ref += *motor_controller->current_feedforward_ptr;
        if (motor_setting->close_loop_type & CURRENT_LOOP)
            pid_ref = PIDCalculate(&motor_controller->current_PID, measure->real_current, pid_ref, 0.001f);

        if (motor_setting->feedback_reverse_flag == FEEDBACK_DIRECTION_REVERSE)
            pid_ref *= -1;

        if (motor->motor_settings.send_mode == SEND_MODE_NORMAL)
            set = (int16_t)pid_ref;
        else
            set = (int16_t)output_external;

        group = motor->sender_group;
        num = motor->message_num;
        sender_tx_buff[group][2 * num] = (uint8_t)(set >> 8);
        sender_tx_buff[group][2 * num + 1] = (uint8_t)(set & 0x00ff);

        if (motor->stop_flag == MOTOR_STOP)
            memset(sender_tx_buff[group] + 2 * num, 0, 2);
    }

    for (uint8_t i = 0; i < SENDER_GROUP_CNT; ++i)
    {
        if (sender_enable_flag[i])
            dji_can_transmit(sender_tx_id[i], sender_tx_buff[i], sender_can_bus[i]);
    }
}
