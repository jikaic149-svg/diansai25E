#ifndef HT04_H
#define HT04_H

/**
 * @file HT04.h
 * @brief 航特HT04电机驱动（MIT协议，CAN抽象）
 * @origin 云台 modules/motor/HTmotor/HT04.h
 */

#include <stdint.h>
#include "../motor_def.h"

#define HT_MOTOR_CNT 4
#define SPEED_BUFFER_SIZE 5
#define HT_SPEED_BIAS -0.0109901428f

#define P_MIN  (-95.5f)
#define P_MAX  95.5f
#define V_MIN  (-45.0f)
#define V_MAX  45.0f
#define T_MIN  (-18.0f)
#define T_MAX  18.0f
#define KP_MIN 0.0f
#define KP_MAX 500.0f
#define KD_MIN 0.0f
#define KD_MAX 5.0f

typedef struct
{
    float total_angle;
    float last_angle;
    float speed_rads;
    float speed_buff[SPEED_BUFFER_SIZE];
    float real_current;
    float feed_dt;
    uint32_t feed_cnt;
} HTMotor_Measure_t;

typedef struct
{
    HTMotor_Measure_t measure;
    Motor_Control_Setting_s motor_settings;
    PIDInstance current_PID;
    PIDInstance speed_PID;
    PIDInstance angle_PID;
    float *other_angle_feedback_ptr;
    float *other_speed_feedback_ptr;
    float *speed_feedforward_ptr;
    float *current_feedforward_ptr;
    float pid_ref;
    Motor_Working_Type_e stop_flag;

    uint32_t tx_id;
    uint32_t rx_id;
    uint8_t  can_bus;
} HTMotorInstance;

typedef enum
{
    CMD_MOTOR_MODE    = 0xfc,
    CMD_RESET_MODE    = 0xfd,
    CMD_ZERO_POSITION = 0xfe,
} HTMotor_Mode_t;

typedef void (*HT_CANTransmit_t)(uint32_t tx_id, const uint8_t *data, uint8_t can_bus);
typedef float (*HT_GetDeltaT_t)(uint32_t *cnt);

void HTMotorRegisterHW(HT_CANTransmit_t can_send, HT_GetDeltaT_t get_dt);

HTMotorInstance *HTMotorInit(Motor_Init_Config_s *config);
void HTMotorSetRef(HTMotorInstance *motor, float ref);
void HTMotorStop(HTMotorInstance *motor);
void HTMotorEnable(HTMotorInstance *motor);
void HTMotorOuterLoop(HTMotorInstance *motor, Closeloop_Type_e type);
void HTMotorSetMode(HTMotor_Mode_t cmd, HTMotorInstance *motor);

void HTMotorDecode(uint32_t rx_id, uint8_t *rx_data, uint8_t can_bus);
void HTMotorControl(void);

#endif
