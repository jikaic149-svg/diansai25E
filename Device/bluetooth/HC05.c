/**
 * @file HC05.c
 * @brief HC05蓝牙通信模块实现
 */

#include "HC05.h"

#define FRAME_HEAD 0xAA
#define FRAME_END  0x55

void HC05_Init(HC05_Instance *inst, HC05_UartSend_t send_fn)
{
    inst->uart_send = send_fn;
    for (int i = 0; i < HC05_DATASIZE; i++)
        inst->recv_data[i] = 0;
}

void HC05_DecodeFrame(HC05_Instance *inst, const uint8_t *rxbuf)
{
    if (rxbuf[0] == FRAME_HEAD && rxbuf[HC05_DATASIZE + 1] == FRAME_END)
    {
        for (int i = 0; i < HC05_DATASIZE; i++)
            inst->recv_data[i] = rxbuf[i + 1];
    }
}

void HC05_SendData(HC05_Instance *inst, const uint8_t *data, uint8_t data_num)
{
    if (!inst->uart_send || data_num > HC05_DATASIZE)
        return;

    inst->send_data[0] = FRAME_HEAD;
    for (int i = 0; i < data_num; i++)
        inst->send_data[i + 1] = data[i];
    inst->send_data[HC05_DATASIZE + 1] = FRAME_END;

    inst->uart_send(inst->send_data, data_num + 2);
}
