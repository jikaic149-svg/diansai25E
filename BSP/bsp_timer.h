/**
 * @file bsp_timer.h
 * @brief BSP 定时器模块
 *        SysTick: 系统时间戳
 *        TIMG0:   Schedule 调度器 tick
 */
#ifndef BSP_TIMER_H
#define BSP_TIMER_H

#include <stdint.h>

#include <stdbool.h>

/**
 * @brief 系统级初始化（SYSCFG + 启动延时）
 */
void BSP_System_Init(void);

/**
 * @brief 全局中断使能
 */
void BSP_System_EnableIRQ(void);

void     BSP_Timer_Init(void);
uint32_t BSP_Timer_GetTickMs(void);

void     BSP_Timer_ScheduleInit(void);
uint16_t BSP_Timer_GetScheduleTick(void);

/**
 * @brief SysTick ISR 内部处理（由 isr.c 调用）
 */
void BSP_Timer_SysTick_Handler(void);

/**
 * @brief Schedule 定时器 ISR 内部处理（由 isr.c 调用）
 */
void BSP_Timer_Schedule_IRQHandler(void);

/**
 * @brief 启动 QEI 采样定时器（TIMG12，1ms 中断）
 */
void BSP_Timer_QEI_Start(void);

/**
 * @brief QEI 采样定时器 ISR 内部处理（由 isr.c 调用）
 */
void BSP_Timer_QEI_IRQHandler(void);

/**
 * @brief 检查编码器更新标志（每 10ms 置位一次）
 */
bool BSP_Timer_IsEncUpdateReady(void);

/**
 * @brief 清除编码器更新标志
 */
void BSP_Timer_ClearEncUpdateFlag(void);

#endif /* BSP_TIMER_H */
