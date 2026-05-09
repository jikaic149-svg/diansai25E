#ifndef HC05_H
#define HC05_H

/**
 * @file HC05.h
 * @brief HC05蓝牙通信模块（UART抽象）
 * @note  协议: [0xAA] [data...] [0x55]
 */

#include <stdint.h>

#define HC05_DATASIZE 4

/**
 * @brief UART发送函数指针
 * @param data 发送数据
 * @param len  数据长度
 */
typedef void (*HC05_UartSend_t)(const uint8_t *data, uint16_t len);

typedef struct
{
    uint8_t send_data[HC05_DATASIZE + 2]; // 加帧头帧尾
    uint8_t recv_data[HC05_DATASIZE];
    HC05_UartSend_t uart_send;
} HC05_Instance;

/**
 * @brief 初始化HC05实例
 * @param inst    实例指针(调用者分配)
 * @param send_fn UART发送函数
 */
void HC05_Init(HC05_Instance *inst, HC05_UartSend_t send_fn);

/**
 * @brief 解析接收到的数据帧
 * @param inst  HC05实例
 * @param rxbuf 接收缓冲区(HC05_DATASIZE+2 字节)
 */
void HC05_DecodeFrame(HC05_Instance *inst, const uint8_t *rxbuf);

/**
 * @brief 发送数据
 * @param inst     HC05实例
 * @param data     数据
 * @param data_num 数据个数(≤HC05_DATASIZE)
 */
void HC05_SendData(HC05_Instance *inst, const uint8_t *data, uint8_t data_num);

#endif
