/**
 * @file  MODE_TASK_SQUARE.h
 * @brief 矩形循迹任务（1m × 1m 场地，4 个内切圆 90° 转向）
 *
 *   流程：上电由 T<n> 命令切入 → STRAIGHT(IR 循迹+里程) → 内切圆弧转向 →
 *        重复 4 次 → 原地停车
 *   规则：恒定基速；起步/停车走斜坡；丢线 > 5s 立即制动
 */
#ifndef MODE_TASK_SQUARE_H
#define MODE_TASK_SQUARE_H

#include <stdint.h>

/** @brief 初始化任务（清状态机/计数器） */
void TaskSquare_Init(void);

/** @brief 周期执行（由 mode_manager 在 5ms 控制环里调用） */
void TaskSquare_Exec(void);

/** @brief 任务是否完成（4 个直角走完并停稳） */
uint8_t TaskSquare_IsDone(void);

#endif /* MODE_TASK_SQUARE_H */
