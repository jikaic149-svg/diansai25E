#ifndef VOFA_H
#define VOFA_H

/**
 * @file vofa.h
 * @brief VOFA+ JustFloat 调试协议（UART抽象）
 */

#include <stdint.h>

/**
 * @brief UART发送函数指针
 * @param data  发送数据
 * @param len   数据长度
 */
typedef void (*VOFA_UartSend_t)(const uint8_t *data, uint16_t len);

/**
 * @brief 注册UART发送函数
 */
void VOFA_RegisterSend(VOFA_UartSend_t send_fn);

/**
 * @brief VOFA JustFloat 协议输出
 * @param data  浮点数数组
 * @param num   浮点数个数
 */
void vofa_justfloat_output(float *data, uint8_t num);

#endif
