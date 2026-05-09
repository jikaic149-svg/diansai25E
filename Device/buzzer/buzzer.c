/**
 * @file    buzzer.c
 * @brief   通用蜂鸣器控制实现
 * @origin  底盘 project/code/extern.c (蜂鸣器部分)
 * @note    通过函数指针解耦GPIO操作, 100%可移植
 */
#include "buzzer.h"

static uint16_t beep_tick_period = 5;   // Tick周期(ms)
static int32_t  beep_countdown = 0;     // 剩余tick数
static buzzer_gpio_func_t gpio_set = 0; // GPIO控制函数

void Buzzer_Init(uint16_t tick_period_ms, buzzer_gpio_func_t gpio_set_func)
{
    beep_tick_period = (tick_period_ms > 0) ? tick_period_ms : 1;
    gpio_set = gpio_set_func;
    beep_countdown = 0;
}

void Buzzer_SetTime(uint16_t time_ms)
{
    if (beep_tick_period > 0)
        beep_countdown = (int32_t)time_ms / beep_tick_period;
}

void Buzzer_Tick(void)
{
    if (!gpio_set) return;

    if (beep_countdown > 0)
    {
        gpio_set(1);
        beep_countdown--;
    }
    else
    {
        gpio_set(0);
    }
}
