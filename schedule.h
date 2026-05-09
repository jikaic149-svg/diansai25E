/**
 * @file schedule.h
 * @brief 任务调度层：对接 simple_task_scheduler，分配各模块到不同时间槽
 */
#ifndef SCHEDULE_H
#define SCHEDULE_H

#include <stdint.h>

/**
 * @brief 调度器初始化（初始化所有模块 + 启动 SysTick）
 */
void Schedule_Init(void);

/**
 * @brief 主循环调度入口（在 while(1) 中调用）
 */
void Schedule_Run(void);

#endif /* SCHEDULE_H */
