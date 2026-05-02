#ifndef BSP_UART_H
#define BSP_UART_H

/**
 * @file    bsp_uart.h
 * @brief   BSP层 — USART1 底层操作 (DMA TX + IDLE中断 RX)
 * @note    引脚: PA9(TX), PA10(RX), 波特率: 115200
 *          唯一包含 HAL 库的层级
 */

#include <stdint.h>
#include <stdbool.h>

#define BSP_UART1_RX_BUF_SIZE  64U
#define BSP_UART1_TX_BUF_SIZE  64U

/**
 * @brief  UART1 接收完成回调 (上层实现)
 * @param  data 接收到的数据指针
 * @param  len  数据长度
 */
typedef void (*BSP_UART1_RxCallback_t)(const uint8_t *data, uint16_t len);

/**
 * @brief  初始化 USART1 (GPIO + USART + DMA + NVIC)
 */
void BSP_UART1_Init(void);

/**
 * @brief  注册接收回调函数
 * @param  callback 回调函数指针
 */
void BSP_UART1_RegisterRxCallback(BSP_UART1_RxCallback_t callback);

/**
 * @brief  DMA 发送数据 (非阻塞)
 * @param  data 数据指针
 * @param  len  数据长度
 * @retval true=启动成功, false=DMA忙
 */
bool BSP_UART1_SendDMA(const uint8_t *data, uint16_t len);

/**
 * @brief  USART1 中断处理 (由 isr / stm32f4xx_it.c 调用)
 */
void BSP_UART1_IRQHandler(void);

/**
 * @brief  DMA2 Stream7 (USART1 TX) 中断处理
 */
void BSP_UART1_DMA_TX_IRQHandler(void);

/**
 * @brief  DMA2 Stream5 (USART1 RX) 中断处理
 */
void BSP_UART1_DMA_RX_IRQHandler(void);

#endif /* BSP_UART_H */
