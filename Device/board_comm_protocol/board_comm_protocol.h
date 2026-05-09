/**
 * @file    board_comm_protocol.h
 * @brief   板间通信协议（双板UART通信, 帧头校验和）
 * @origin  底盘 project/code/board_comm + 云台 modules/timachine/board_comm
 * @note    本文件只包含协议逻辑, 不含UART驱动
 *          用户需实现: board_comm_uart_send_byte(uint8_t byte) 发送单字节
 *
 * 帧格式(22字节):
 *   [0x55][0xAA][int32×2][float×2][checksum(4B)]
 */
#ifndef BOARD_COMM_PROTOCOL_H
#define BOARD_COMM_PROTOCOL_H

#include <stdint.h>

#define BOARD_COMM_FRAME_SIZE   22u     // 帧总大小
#define BOARD_COMM_DATA_SIZE    16u     // 数据部分大小
#define BOARD_COMM_HEADER_0     0x55
#define BOARD_COMM_HEADER_1     0xAA

/* 通信数据包(packed, 可直接内存映射) */
#ifdef __GNUC__
typedef struct __attribute__((packed))
#else
#pragma pack(push, 1)
typedef struct
#endif
{
    uint8_t  header[2];         // 帧头 0x55 0xAA
    int32_t  int_data[2];       // 2个int32
    float    float_data[2];     // 2个float
    uint32_t checksum;          // 校验和
} BoardCommPacket_t;
#ifndef __GNUC__
#pragma pack(pop)
#endif

/* 收发数据缓存 */
typedef struct
{
    int32_t tx_int_data[2];
    float   tx_float_data[2];
    int32_t rx_int_data[2];
    float   rx_float_data[2];
} BoardCommData_t;

/* UART发送函数类型(用户实现) */
typedef void (*board_comm_send_byte_func_t)(uint8_t byte);

/**
 * @brief 初始化板间通信协议
 * @param send_byte_func  UART发送单字节的函数指针
 */
void BoardComm_Init(board_comm_send_byte_func_t send_byte_func);

/**
 * @brief 发送状态数据(2个int32 + 2个float)
 * @param int_data   整型数据数组[2]
 * @param float_data 浮点数据数组[2]
 */
void BoardComm_Send(int32_t int_data[2], float float_data[2]);

/**
 * @brief 解析接收到的数据帧
 * @param buffer  接收到的原始字节(长度应为 BOARD_COMM_FRAME_SIZE)
 * @return 0=校验失败, 1=解析成功
 */
uint8_t BoardComm_Decode(const uint8_t *buffer);

/**
 * @brief 获取接收到的整型数据
 */
int32_t *BoardComm_GetIntData(void);

/**
 * @brief 获取接收到的浮点数据
 */
float *BoardComm_GetFloatData(void);

#endif /* BOARD_COMM_PROTOCOL_H */
