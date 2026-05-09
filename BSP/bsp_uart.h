#ifndef BSP_UART_H
#define BSP_UART_H

#include "ti_msp_dl_config.h"
#include <stdint.h>
#include <stdbool.h>

#define RX_BUFFER_SIZE  256

extern volatile uint8_t g_wit_rx_buffer[RX_BUFFER_SIZE];
extern volatile uint8_t g_bt_rx_buffer[RX_BUFFER_SIZE];
extern volatile uint8_t g_track_rx_buffer[RX_BUFFER_SIZE];

void BSP_UART_Init(void);

void BSP_BT_SendPacket(uint8_t *data, uint16_t len);
void BSP_Track_SendPacket(uint8_t *data, uint16_t len);

/**
 * @brief UART 中断内部处理（由 isr.c 调用）
 */
void BSP_UART_WIT_IRQHandler(void);
void BSP_UART_Blue_IRQHandler(void);
void BSP_UART_Track_IRQHandler(void);

extern void BSP_WIT_ProcessPacket(uint8_t *data, uint16_t len);
extern void BSP_BT_ProcessPacket(uint8_t *data, uint16_t len);
extern void BSP_Track_ProcessPacket(uint8_t *data, uint16_t len);

#endif /* BSP_UART_H */
