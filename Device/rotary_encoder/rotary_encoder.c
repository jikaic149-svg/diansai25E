/**
 * @file    rotary_encoder.c
 * @brief   解析并更新旋钮模块逻辑
 */
#include "rotary_encoder.h"

void RotaryEncoder_Init(RotaryEncoder_t *encoder, RotaryEncoder_Init_Config_t *config)
{
    if (encoder == 0 || config == 0) return;
    
    encoder->GetPulseDiff = config->GetPulseDiff;
    encoder->TotalValue = 0;
    encoder->StepValue = 0;
    
    /* 联合初始化绑定的内置按钮模块 */
    Button_Init(&encoder->KnobButton, &config->ButtonConfig);
}

void RotaryEncoder_Update(RotaryEncoder_t *encoder, float dt_s)
{
    if (!encoder) return;
    
    /* 如果挂载了获取旋转差值的底层回调，则进行更新 */
    if (encoder->GetPulseDiff) {
        encoder->StepValue = encoder->GetPulseDiff();
        encoder->TotalValue += encoder->StepValue;
    }
    
    /* 更新内部绑定的按钮状态，执行纯软消抖时间运算 */
    Button_Update(&encoder->KnobButton, dt_s);
}

int32_t RotaryEncoder_GetTotalValue(RotaryEncoder_t *encoder)
{
    return encoder->TotalValue;
}

int16_t RotaryEncoder_GetStep(RotaryEncoder_t *encoder)
{
    int16_t current_step = encoder->StepValue;
    /* 取走操作即被消耗，将步进置 0 防止 UI 重复叠加 */
    encoder->StepValue = 0; 
    return current_step;
}
