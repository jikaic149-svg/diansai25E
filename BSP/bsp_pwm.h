/**
 * @file bsp_pwm.h
 * @brief BSP PWM 模块：TIMG7 双通道 PWM 输出（纯整数接口，零浮点）
 */
#ifndef BSP_PWM_H
#define BSP_PWM_H

#include <stdint.h>

#define PWM_CHANNEL_0   0
#define PWM_CHANNEL_1   1

/**
 * @brief 初始化 TIMG7 双通道 PWM
 */
void BSP_PWM_Init(void);

/**
 * @brief 设置指定通道的占空比（纯整数运算）
 * @param channel   PWM_CHANNEL_0 或 PWM_CHANNEL_1
 * @param duty      占空比值，范围 0 ~ max_duty
 * @param max_duty  占空比满量程（如 1000 代表 duty 范围 0~1000）
 */
void BSP_PWM_SetDuty(uint8_t channel, uint16_t duty, uint16_t max_duty);

/**
 * @brief 同时设置两路占空比
 */
void BSP_PWM_SetDutyBoth(uint16_t duty0, uint16_t duty1, uint16_t max_duty);

#endif /* BSP_PWM_H */
