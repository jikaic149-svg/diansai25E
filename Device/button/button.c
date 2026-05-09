/**
 * @file    button.c
 * @brief   按键消抖模块的纯软逻辑实现
 */
#include "button.h"

#define DEBOUNCE_TIME_S     0.02f   // 20ms 防抖
#define LONG_PRESS_TIME_S   0.8f    // 0.8s 算长按

void Button_Init(Button_t *btn, Button_Init_Config_t *config)
{
    if (btn == 0 || config == 0) return;
    
    btn->ReadPin = config->ReadPin;
    btn->ActiveLevel = config->ActiveLevel;
    
    btn->IsPressed = 0;
    btn->LongPressTriggered = 0;
    btn->PressTime = 0.0f;
    btn->Event = BUTTON_EVENT_NONE;
}

void Button_Update(Button_t *btn, float dt_s)
{
    if (!btn || !btn->ReadPin) return;

    /* 获取底层引脚电平 */
    uint8_t current_level = btn->ReadPin();
    
    if (current_level == btn->ActiveLevel) 
    {
        btn->PressTime += dt_s;
        
        /* 防抖处理确认按下 */
        if (btn->PressTime > DEBOUNCE_TIME_S && !btn->IsPressed) {
            btn->IsPressed = 1;
            btn->LongPressTriggered = 0;
        }
        
        /* 长按检测识别 */
        if (btn->IsPressed && btn->PressTime > LONG_PRESS_TIME_S && !btn->LongPressTriggered) {
            btn->LongPressTriggered = 1;
            btn->Event = BUTTON_EVENT_LONG_PRESS;
        }
    } 
    else 
    {   
        /* 松开处理 */
        if (btn->IsPressed) {
            if (!btn->LongPressTriggered && btn->PressTime > DEBOUNCE_TIME_S) {
                btn->Event = BUTTON_EVENT_SHORT_PRESS;
            } else {
                btn->Event = BUTTON_EVENT_RELEASE;
            }
        }
        
        /* 彻底清除并重置状态 */
        btn->IsPressed = 0;
        btn->LongPressTriggered = 0;
        btn->PressTime = 0.0f;
    }
}

ButtonEvent_e Button_GetEvent(Button_t *btn)
{
    return btn->Event;
}

void Button_ClearEvent(Button_t *btn)
{
    btn->Event = BUTTON_EVENT_NONE;
}
