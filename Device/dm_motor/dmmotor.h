#ifndef DMMOTOR_H
#define DMMOTOR_H

/**
 * @file dmmotor.h
 * @brief 达妙(DM)电机驱动（MIT协议，CAN抽象）
 * @origin 云台 modules/motor/DMmotor/dmmotor.h
 */

#include <stdint.h>
#include "../motor_def.h"

#define DM_MOTOR_CNT 4

#define DM_P_MIN  (-12.56637f)
#define DM_P_MAX  12.56637f
#define DM_V_MIN  (-45.0f)
#define DM_V_MAX  45.0f
#define DM_T_MIN  (-18.0f)
#define DM_T_MAX  18.0f

typedef struct
{
    uint8_t id;
    uint8_t state;
    float velocity;
    float last_position;
    float position;
    float torque;
    float T_Mos;
    float T_Rotor;
    int32_t total_round;
    float total_angle;
} DM_Motor_Measure_s;

typedef struct
{
    uint16_t position_des;
    uint16_t velocity_des;
    uint16_t torque_des;
    uint16_t Kp;
    uint16_t Kd;
} DMMotor_Send_s;

typedef struct
{
    DM_Motor_Measure_s measure;
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

    /* CAN配置 */
    uint32_t tx_id;
    uint32_t rx_id;
    uint8_t  can_bus;
} DMMotorInstance;

typedef enum
{
    DM_CMD_MOTOR_MODE   = 0xfc,
    DM_CMD_RESET_MODE   = 0xfd,
    DM_CMD_ZERO_POSITION = 0xfe,
    DM_CMD_CLEAR_ERROR  = 0xfb,
} DMMotor_Mode_e;

/**
 * @brief CAN发送函数指针
 */
typedef void (*DM_CANTransmit_t)(uint32_t tx_id, const uint8_t *data, uint8_t can_bus);

/**
 * @brief 注册CAN发送函数
 */
void DMMotorRegisterHW(DM_CANTransmit_t can_send);

/**
 * @brief 初始化DM电机
 */
DMMotorInstance *DMMotorInit(Motor_Init_Config_s *config);

void DMMotorSetRef(DMMotorInstance *motor, float ref);
void DMMotorOuterLoop(DMMotorInstance *motor, Closeloop_Type_e closeloop_type);
void DMMotorEnable(DMMotorInstance *motor);
void DMMotorStop(DMMotorInstance *motor);
void DMMotorSetMode(DMMotor_Mode_e mode, DMMotorInstance *motor);

/**
 * @brief CAN接收回调
 */
void DMMotorDecode(uint32_t rx_id, uint8_t *rx_data, uint8_t can_bus);

/**
 * @brief 计算PID并发送控制帧(需周期调用)
 */
void DMMotorControl(void);

#endif
