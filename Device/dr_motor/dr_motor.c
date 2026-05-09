/**
 * @file    dr_motor.c
 * @brief   电机通用驱动功能实现
 */
#include "dr_motor.h"
#include <stdlib.h>

void DR_Motor_Init(DR_Motor_t *motor, DR_Motor_Init_Config_t *config)
{
    if (!motor || !config) return;
    
    motor->SetPWM = config->SetPWM;
    motor->SetDir = config->SetDir;
    motor->max_pwm_period = config->max_pwm_period;
    motor->deadzone = config->deadzone;
    motor->current_dir = 0;
    
    if (motor->SetDir) motor->SetDir(0);
    if (motor->SetPWM) motor->SetPWM(0);
}

void DR_Motor_SetSpeed(DR_Motor_t *motor, int16_t speed)
{
    if (!motor) return;
    
    /* 前馈死区补偿：只有在非 0 速度命令时，才施加底准摩擦起步电压 */
    if (motor->deadzone > 0) {
        if (speed > 0) {
            speed += (int16_t)motor->deadzone;
        } else if (speed < 0) {
            speed -= (int16_t)motor->deadzone;
        }
    }

    /* 1. PWM 占空比计算与软双重限幅保护 */
    uint16_t pwm_duty = (uint16_t)abs(speed);
    if (pwm_duty >= motor->max_pwm_period) {
        pwm_duty = motor->max_pwm_period - 1;
    }
    
    /* 2. 计算目标方向 */
    int8_t new_dir = (speed > 0) ? 1 : (speed < 0) ? -1 : 0;
    
    /* 3. 设置方向控制（仅在方向改变时更新） */
    if (new_dir != motor->current_dir)
    {
        /* 【安全死区】换向前先刹车断电，防止 H 桥直通击穿 */
        if (motor->SetPWM) motor->SetPWM(0);
        
        /* 改变硬件方向引脚 */
        if (motor->SetDir) motor->SetDir(new_dir);
        motor->current_dir = new_dir;
    }
    
    /* 4. 最后再输出新的 PWM 占空比 */
    if (motor->SetPWM) motor->SetPWM(pwm_duty);
}

void DR_Motor_Stop(DR_Motor_t *motor)
{
    DR_Motor_SetSpeed(motor, 0);
}
