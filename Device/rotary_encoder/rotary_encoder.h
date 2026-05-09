/**
 * @file    rotary_encoder.h
 * @brief   OOP 架构通用旋钮交互模块 (包含旋转和按键)
 * @note    不依赖内部 TIM，通过回调获取变化量，并内嵌了 Button 模块处理压感
 */
#ifndef ROTARY_ENCODER_H
#define ROTARY_ENCODER_H

#include <stdint.h>
#include "../button/button.h"

/**
 * @brief 提供当前循环中定时器（或中断）产生的计步差值 (负数代表反转)
 */
typedef int16_t (*Rotary_GetPulseDiff_Func_t)(void);

/**
 * @brief 旋钮编码器实例结构体
 */
typedef struct {
    Rotary_GetPulseDiff_Func_t GetPulseDiff;
    Button_t                   KnobButton;    // 旋钮自身附带的按下按键
    
    int32_t                    TotalValue;    // 全局累加值
    int16_t                    StepValue;     // 单次更新读取的值
} RotaryEncoder_t;

/**
 * @brief 旋钮编码器初始化配置结构体
 */
typedef struct {
    Rotary_GetPulseDiff_Func_t GetPulseDiff;
    Button_Init_Config_t       ButtonConfig;  // 按键参数配置联通
} RotaryEncoder_Init_Config_t;

/* API 声明 */
void RotaryEncoder_Init(RotaryEncoder_t *encoder, RotaryEncoder_Init_Config_t *config);
void RotaryEncoder_Update(RotaryEncoder_t *encoder, float dt_s);
int32_t RotaryEncoder_GetTotalValue(RotaryEncoder_t *encoder);
int16_t RotaryEncoder_GetStep(RotaryEncoder_t *encoder);

#endif /* ROTARY_ENCODER_H */
