#ifndef CAN_COMM_H
#define CAN_COMM_H

/**
 * @file can_comm.h
 * @brief 多机CAN通信收发模块（CAN抽象）
 * @note  协议: [header 's'] [datalen] [payload...] [crc8] [tail 'e']
 */

#include <stdint.h>

#define CAN_COMM_MAX_BUFFSIZE 60
#define CAN_COMM_HEADER 's'
#define CAN_COMM_TAIL   'e'
#define CAN_COMM_OFFSET_BYTES 4 // header + datalen + crc8 + tail

/**
 * @brief CAN发送函数指针(单帧)
 * @param tx_id   发送ID
 * @param data    数据(≤8字节)
 * @param len     数据长度
 * @return 0=成功
 */
typedef int (*CANComm_SendFunc_t)(uint32_t tx_id, const uint8_t *data, uint8_t len);

#pragma pack(1)
typedef struct
{
    /* 发送 */
    uint32_t tx_id;
    uint8_t send_data_len;
    uint8_t send_buf_len;
    uint8_t raw_sendbuf[CAN_COMM_MAX_BUFFSIZE + CAN_COMM_OFFSET_BYTES];
    /* 接收 */
    uint8_t recv_data_len;
    uint8_t recv_buf_len;
    uint8_t raw_recvbuf[CAN_COMM_MAX_BUFFSIZE + CAN_COMM_OFFSET_BYTES];
    uint8_t unpacked_recv_data[CAN_COMM_MAX_BUFFSIZE];
    /* 接收状态 */
    uint8_t recv_state;
    uint8_t cur_recv_len;
    uint8_t update_flag;
    /* 硬件回调 */
    CANComm_SendFunc_t can_send;
} CANCommInstance;
#pragma pack()

typedef struct
{
    uint32_t tx_id;
    uint8_t send_data_len;
    uint8_t recv_data_len;
    CANComm_SendFunc_t can_send;
} CANComm_Init_Config_s;

/**
 * @brief 初始化CANComm
 */
void CANCommInit(CANCommInstance *inst, const CANComm_Init_Config_s *config);

/**
 * @brief 发送数据(自动分包为CAN帧)
 */
void CANCommSend(CANCommInstance *inst, const uint8_t *data);

/**
 * @brief CAN接收回调，由BSP层的CAN接收中断中调用
 * @param inst    CANComm实例
 * @param rx_data 接收到的CAN帧数据
 * @param rx_len  帧长度(≤8)
 */
void CANCommRxCallback(CANCommInstance *inst, const uint8_t *rx_data, uint8_t rx_len);

/**
 * @brief 获取已解包的数据指针
 * @return unpacked_recv_data
 */
void *CANCommGet(CANCommInstance *inst);

#endif
