/**
 * @file bsp_tb6612.h
 * @brief BSP TB6612 电机驱动 GPIO 控制（方向引脚 + STBY 使能）
 */
#ifndef BSP_TB6612_H
#define BSP_TB6612_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief TB6612 全局使能/关断（STBY 引脚）
 */
void BSP_TB6612_SetEnable(bool enable);

/**
 * @brief 左电机方向控制（AIN1/AIN2）
 * @param dir  1=正转, -1=反转, 0=停止
 */
void BSP_TB6612_SetDirLeft(int8_t dir);

/**
 * @brief 右电机方向控制（BIN1/BIN2）
 * @param dir  1=正转, -1=反转, 0=停止
 */
void BSP_TB6612_SetDirRight(int8_t dir);

#endif /* BSP_TB6612_H */
