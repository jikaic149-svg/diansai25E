/**
 * @file    motor_def.h
 * @brief   电机通用数据结构定义（闭环类型、前馈、控制器配置等）
 * @origin  云台 modules/motor/motor_def.h
 * @note    100%可移植, 纯数据类型定义
 *          本文件依赖 pid_controller.h 中的 PIDInstance / PID_Init_Config_s
 */
#ifndef MOTOR_DEF_H
#define MOTOR_DEF_H

#include <stdint.h>
#include "../Application/algorithm/pid_controller.h"

#define LIMIT_MIN_MAX(x, min, max) (x) = (((x) <= (min)) ? (min) : (((x) >= (max)) ? (max) : (x)))

/* 闭环类型(可位或组合) */
typedef enum
{
    OPEN_LOOP = 0b0000,
    CURRENT_LOOP = 0b0001,
    SPEED_LOOP = 0b0010,
    ANGLE_LOOP = 0b0100,
    SPEED_AND_CURRENT_LOOP = 0b0011,
    ANGLE_AND_SPEED_LOOP = 0b0110,
    ALL_THREE_LOOP = 0b0111,
} Closeloop_Type_e;

/* 前馈类型 */
typedef enum
{
    FEEDFORWARD_NONE = 0b00,
    CURRENT_FEEDFORWARD = 0b01,
    SPEED_FEEDFORWARD = 0b10,
    CURRENT_AND_SPEED_FEEDFORWARD = CURRENT_FEEDFORWARD | SPEED_FEEDFORWARD,
} Feedforward_Type_e;

/* 反馈来源 */
typedef enum { MOTOR_FEED = 0, OTHER_FEED } Feedback_Source_e;

/* 电机正反转标志 */
typedef enum { MOTOR_DIRECTION_NORMAL = 0, MOTOR_DIRECTION_REVERSE = 1 } Motor_Reverse_Flag_e;

/* 反馈方向 */
typedef enum { FEEDBACK_DIRECTION_NORMAL = 0, FEEDBACK_DIRECTION_REVERSE = 1 } Feedback_Reverse_Flag_e;

/* 电机工作状态 */
typedef enum { MOTOR_STOP = 0, MOTOR_ENABLED = 1 } Motor_Working_Type_e;

/* 发送模式 */
typedef enum { SEND_MODE_NORMAL = 0, SEND_MODE_MANUAL = 1 } Motor_Send_Mode_e;

/* 电机控制设置 */
typedef struct
{
    Closeloop_Type_e outer_loop_type;
    Closeloop_Type_e close_loop_type;
    Motor_Reverse_Flag_e motor_reverse_flag;
    Feedback_Reverse_Flag_e feedback_reverse_flag;
    Feedback_Source_e angle_feedback_source;
    Feedback_Source_e speed_feedback_source;
    uint8_t feedforward_flag;
    Motor_Send_Mode_e send_mode;
} Motor_Control_Setting_s;

/* 电机控制器（三环PID + 外部反馈） */
typedef struct
{
    float *other_angle_feedback_ptr;
    float *other_speed_feedback_ptr;
    float *speed_feedforward_ptr;
    float *current_feedforward_ptr;

    PIDInstance current_PID;
    PIDInstance speed_PID;
    PIDInstance angle_PID;

    float pid_ref;
    float output_external;
} Motor_Controller_s;

/* 电机类型枚举(根据实际使用的电机修改) */
typedef enum
{
    MOTOR_TYPE_NONE = 0,
    GM6020,      // DJI GM6020
    M3508,       // DJI M3508
    M2006,       // DJI M2006
    LK9025,      // 立科9025
    HT04,        // 航特HT04
    GM6020_NEW,  // 新版固件6020
} Motor_Type_e;

/* 电机控制器初始化配置 */
typedef struct
{
    float *other_angle_feedback_ptr;
    float *other_speed_feedback_ptr;
    float *speed_feedforward_ptr;
    float *current_feedforward_ptr;

    PID_Init_Config_s current_PID;
    PID_Init_Config_s speed_PID;
    PID_Init_Config_s angle_PID;
} Motor_Controller_Init_s;

/* ============= CAN 抽象(用于电机初始化) ============= */

/**
 * @brief CAN发送函数指针类型
 * @param tx_id   发送ID
 * @param data    8字节数据
 * @param can_bus CAN总线编号(0/1/2)
 * @return 0=成功
 */
typedef int (*CAN_SendFunc_t)(uint32_t tx_id, const uint8_t *data, uint8_t can_bus);

/**
 * @brief CAN注册回调函数类型
 * @param rx_id   接收ID
 * @param can_bus CAN总线编号
 * @param callback 回调函数，参数为(rx_id, rx_data[8], user_data)
 * @param user_data 用户数据指针
 * @return 0=成功
 */
typedef int (*CAN_RegisterFunc_t)(uint32_t rx_id, uint8_t can_bus,
    void (*callback)(uint32_t rx_id, uint8_t *rx_data, void *user_data),
    void *user_data);

/* CAN 初始化配置（抽象，不依赖BSP） */
typedef struct
{
    uint32_t tx_id;
    uint32_t rx_id;
    uint8_t  can_bus;   // CAN总线编号(0/1/2)
} CAN_Init_Config_s;

/* 电机初始化配置（通用） */
typedef struct
{
    Motor_Controller_Init_s controller_param_init_config;
    Motor_Control_Setting_s controller_setting_init_config;
    Motor_Type_e motor_type;
    CAN_Init_Config_s can_init_config;
} Motor_Init_Config_s;

#endif /* MOTOR_DEF_H */
