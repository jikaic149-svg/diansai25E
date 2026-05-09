#ifndef SUPER_CAP_H
#define SUPER_CAP_H

/**
 * @file super_cap.h
 * @brief 超级电容模块（CAN抽象）
 */

#include <stdint.h>

#pragma pack(1)
typedef struct
{
    uint16_t vol;     // 电压
    uint16_t current; // 电流
    uint16_t power;   // 功率
} SuperCap_Msg_s;
#pragma pack()

/**
 * @brief CAN发送函数指针
 * @param tx_id CAN发送ID
 * @param data  8字节数据
 */
typedef void (*SuperCap_CANSend_t)(uint32_t tx_id, const uint8_t *data);

typedef struct
{
    uint32_t tx_id;
    SuperCap_Msg_s cap_msg;
    SuperCap_CANSend_t can_send;
} SuperCapInstance;

/**
 * @brief 初始化
 * @param inst    实例(调用者分配)
 * @param tx_id   CAN发送ID
 * @param send_fn CAN发送函数
 */
void SuperCapInit(SuperCapInstance *inst, uint32_t tx_id, SuperCap_CANSend_t send_fn);

/**
 * @brief 解析CAN接收数据(8字节)
 */
void SuperCapDecode(SuperCapInstance *inst, const uint8_t *rxbuff);

/**
 * @brief 发送超级电容控制数据
 */
void SuperCapSend(SuperCapInstance *inst, const uint8_t *data);

/**
 * @brief 获取超级电容信息
 */
SuperCap_Msg_s SuperCapGet(SuperCapInstance *inst);

#endif
