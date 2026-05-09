/**
 * @file    dr_motor.h
 * @brief   OOP 架构电机通用驱动模块
 * @note    剥离固定的外设依赖，提供纯逻辑控制与状态管理（支持限幅与方向缓存）
 */
#ifndef DR_MOTOR_H
#define DR_MOTOR_H

#include <stdint.h>

/**
 * @brief 电机驱动底层控制回调函数定义
 */
typedef void (*DR_Motor_SetPWM_Func_t)(uint16_t pwm_duty);
typedef void (*DR_Motor_SetDir_Func_t)(int8_t dir); /* 1: 正转, -1: 反转, 0: 停止/刹车 */

/**
 * @brief 电机驱动对象结构体
 */
typedef struct {
    DR_Motor_SetPWM_Func_t SetPWM;
    DR_Motor_SetDir_Func_t SetDir;
    
    uint16_t max_pwm_period;
    uint16_t deadzone;          /* 起步死区補偿（由上层配置传入） */
    
    /* 运行时状态缓存，减少不必要的 IO 操作 */
    int8_t current_dir;
} DR_Motor_t;

/**
 * @brief 电机驱动初始化配置结构体
 */
typedef struct {
    DR_Motor_SetPWM_Func_t SetPWM;
    DR_Motor_SetDir_Func_t SetDir;
    uint16_t max_pwm_period;
    uint16_t deadzone;          /* 摩擦力起步补偿 */
} DR_Motor_Init_Config_t;

/* ======= API 声明 ======= */
void DR_Motor_Init(DR_Motor_t *motor, DR_Motor_Init_Config_t *config);
void DR_Motor_SetSpeed(DR_Motor_t *motor, int16_t speed);
void DR_Motor_Stop(DR_Motor_t *motor);

#endif /* DR_MOTOR_H */
