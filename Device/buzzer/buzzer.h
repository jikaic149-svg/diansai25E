/**
 * @file    buzzer.h
 * @brief   通用蜂鸣器控制（定时触发模式）
 * @origin  底盘 project/code/extern.c (蜂鸣器部分)
 * @note    用户需实现 buzzer_gpio_set(uint8_t level) 函数对接硬件GPIO
 *
 * 使用方法:
 *   1. 实现 buzzer_gpio_set() 并注册
 *   2. 调用 Buzzer_Init(周期ms)
 *   3. 需要蜂鸣时调用 Buzzer_SetTime(ms)
 *   4. 在定时中断中调用 Buzzer_Tick()
 */
#ifndef BUZZER_H
#define BUZZER_H

#include <stdint.h>

/* 用户需实现的GPIO控制函数类型 */
typedef void (*buzzer_gpio_func_t)(uint8_t level);

/**
 * @brief 初始化蜂鸣器
 * @param tick_period_ms  Buzzer_Tick() 的调用周期(ms)
 * @param gpio_set_func   GPIO设置函数指针(用户实现: level=1响, level=0停)
 */
void Buzzer_Init(uint16_t tick_period_ms, buzzer_gpio_func_t gpio_set_func);

/**
 * @brief 设置蜂鸣持续时间
 * @param time_ms 蜂鸣时长(毫秒)
 */
void Buzzer_SetTime(uint16_t time_ms);

/**
 * @brief 蜂鸣器节拍处理, 放入定时中断或定时任务中调用
 */
void Buzzer_Tick(void);

#endif /* BUZZER_H */
