#ifndef LK9025_H
#define LK9025_H

/**
 * @file LK9025.h
 * @brief 瓴控LK9025电机驱动（多电机CAN指令，CAN抽象）
 * @origin 云台 modules/motor/LKmotor/LK9025.h
 */

#include <stdint.h>
#include "../motor_def.h"

#define LK_MOTOR_MX_CNT 4

#define I_MIN  (-2000)
#define I_MAX  2000
#define ECD_ANGLE_COEF_LK (360.0f / 65536.0f)
#define CURRENT_TORQUE_COEF_LK 0.003645f

typedef struct
{
    uint16_t last_ecd;
    uint16_t ecd;
    float angle_single_round;
    float speed_rads;
    int16_t real_current;
    uint8_t temperature;

    float total_angle;
    int32_t total_round;

    float feed_dt;
    uint32_t feed_dwt_cnt;
} LKMotor_Measure_t;

typedef struct
{
    LKMotor_Measure_t measure;
    Motor_Control_Setting_s motor_settings;
    float *other_angle_feedback_ptr;
    float *other_speed_feedback_ptr;
    float *speed_feedforward_ptr;
    float *current_feedforward_ptr;
    PIDInstance current_PID;
    PIDInstance speed_PID;
    PIDInstance angle_PID;
    float pid_ref;
    Motor_Working_Type_e stop_flag;

    uint32_t tx_id;
    uint32_t rx_id;
    uint8_t  can_bus;
    uint8_t  motor_idx; // 该电机在多电机发送中的索引(0~3)
} LKMotorInstance;

typedef void (*LK_CANTransmit_t)(uint32_t tx_id, const uint8_t *data, uint8_t can_bus);
typedef float (*LK_GetDeltaT_t)(uint32_t *cnt);

void LKMotorRegisterHW(LK_CANTransmit_t can_send, LK_GetDeltaT_t get_dt);

LKMotorInstance *LKMotorInit(Motor_Init_Config_s *config);
void LKMotorSetRef(LKMotorInstance *motor, float ref);
void LKMotorStop(LKMotorInstance *motor);
void LKMotorEnable(LKMotorInstance *motor);

void LKMotorDecode(uint32_t rx_id, uint8_t *rx_data, uint8_t can_bus);
void LKMotorControl(void);

#endif
