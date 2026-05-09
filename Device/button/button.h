/**
 * @file    button.h
 * @brief   OOP 架构通用按键消抖模块 (100%可移植)
 * @note    不依赖任何特定芯片库，全部通过函数指针注入管脚读取逻辑
 */
#ifndef BUTTON_H
#define BUTTON_H

#include <stdint.h>

/**
 * @brief 按键事件枚举
 */
typedef enum {
    BUTTON_EVENT_NONE = 0,
    BUTTON_EVENT_SHORT_PRESS,
    BUTTON_EVENT_LONG_PRESS,
    BUTTON_EVENT_RELEASE
} ButtonEvent_e;

/**
 * @brief 读取真实电平的底层函数指针
 */
typedef uint8_t (*Button_ReadPin_Func_t)(void);

/**
 * @brief 按键模块实例结构体
 */
typedef struct {
    Button_ReadPin_Func_t ReadPin;
    uint8_t               ActiveLevel;

    uint8_t               IsPressed;
    uint8_t               LongPressTriggered;
    float                 PressTime;
    ButtonEvent_e         Event;
} Button_t;

/**
 * @brief 按键模块初始化配置结构体
 */
typedef struct {
    Button_ReadPin_Func_t ReadPin;
    uint8_t               ActiveLevel;
} Button_Init_Config_t;

/* API 声明 */
void Button_Init(Button_t *btn, Button_Init_Config_t *config);
void Button_Update(Button_t *btn, float dt_s);
ButtonEvent_e Button_GetEvent(Button_t *btn);
void Button_ClearEvent(Button_t *btn);

#endif /* BUTTON_H */
