/**
 * @file  bsp_encoder.h
 * @brief BSP 编码器驱动（软件 4 倍频正交解码，GPIO 中断计数）
 *        支持两路电机编码器，通过 GPIOB 中断实现
 */
#ifndef BSP_ENCODER_H
#define BSP_ENCODER_H

#include <stdint.h>

/**
 * @brief 初始化编码器 GPIO 中断（在 BSP_System_EnableIRQ 之前调用）
 */
void BSP_Encoder_Init(void);

/**
 * @brief GPIOB 中断处理（由 isr.c 调用）
 *        内部自动判断触发引脚并更新对应计数器
 */
void BSP_Encoder_GPIOB_IRQHandler(void);

/**
 * @brief 原子读取左轮编码器脉冲差值并清零
 * @return 自上次调用以来的脉冲增量（正=正转，负=反转）
 */
int16_t BSP_Encoder_GetLeftDiff(void);

/**
 * @brief 原子读取右轮编码器脉冲差值并清零
 * @return 自上次调用以来的脉冲增量（正=正转，负=反转）
 */
int16_t BSP_Encoder_GetRightDiff(void);

#endif /* BSP_ENCODER_H */
