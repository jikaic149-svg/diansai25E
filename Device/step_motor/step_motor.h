#ifndef STEP_MOTOR_H
#define STEP_MOTOR_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief 步进电机UART发送函数类型定义
 * @param data 待发送数据指针
 * @param len  数据长度
 * @note  用户需根据平台实现此函数(如 HAL_UART_Transmit_DMA)
 */
typedef void (*StepMotor_UartSend_t)(uint8_t *data, uint16_t len);

/**
 * @brief 注册步进电机UART发送回调（按电机地址分组）
 * @param group   UART分组编号(0或1)
 * @param send_fn 发送函数指针
 */
void StepMotor_RegisterUart(uint8_t group, StepMotor_UartSend_t send_fn);

/**
 * @brief 设置电机地址与UART分组的映射
 * @param addr  电机地址
 * @param group UART分组编号(0或1)
 */
void StepMotor_SetAddrGroup(uint8_t addr, uint8_t group);

/**
 * @brief 步进电机控制命令发送(由X_V2协议层调用)
 */
void step_sendcmd(uint8_t addr, uint8_t *cmd, uint16_t len);

#endif
