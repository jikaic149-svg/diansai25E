/**
 * @file can_comm.c
 * @brief 多机CAN通信收发模块实现
 */

#include "can_comm.h"
#include "../Application/algorithm/crc8.h"
#include <string.h>

static void CANCommResetRx(CANCommInstance *inst)
{
    memset(inst->raw_recvbuf, 0, inst->cur_recv_len);
    inst->recv_state = 0;
    inst->cur_recv_len = 0;
}

void CANCommInit(CANCommInstance *inst, const CANComm_Init_Config_s *config)
{
    memset(inst, 0, sizeof(CANCommInstance));
    inst->tx_id = config->tx_id;
    inst->recv_data_len = config->recv_data_len;
    inst->recv_buf_len = config->recv_data_len + CAN_COMM_OFFSET_BYTES;
    inst->send_data_len = config->send_data_len;
    inst->send_buf_len = config->send_data_len + CAN_COMM_OFFSET_BYTES;
    inst->can_send = config->can_send;

    inst->raw_sendbuf[0] = CAN_COMM_HEADER;
    inst->raw_sendbuf[1] = config->send_data_len;
    inst->raw_sendbuf[config->send_data_len + CAN_COMM_OFFSET_BYTES - 1] = CAN_COMM_TAIL;
}

void CANCommSend(CANCommInstance *inst, const uint8_t *data)
{
    if (!inst->can_send) return;

    memcpy(inst->raw_sendbuf + 2, data, inst->send_data_len);
    inst->raw_sendbuf[2 + inst->send_data_len] = crc_8(inst->raw_sendbuf + 2, inst->send_data_len);

    uint8_t send_len;
    for (uint8_t i = 0; i < inst->send_buf_len; i += 8)
    {
        send_len = (inst->send_buf_len - i >= 8) ? 8 : (inst->send_buf_len - i);
        inst->can_send(inst->tx_id, inst->raw_sendbuf + i, send_len);
    }
}

void CANCommRxCallback(CANCommInstance *inst, const uint8_t *rx_data, uint8_t rx_len)
{
    if (rx_data[0] == CAN_COMM_HEADER && inst->recv_state == 0)
    {
        if (rx_data[1] == inst->recv_data_len)
            inst->recv_state = 1;
        else
            return;
    }

    if (inst->recv_state)
    {
        if (inst->cur_recv_len + rx_len > inst->recv_buf_len)
        {
            CANCommResetRx(inst);
            return;
        }

        memcpy(inst->raw_recvbuf + inst->cur_recv_len, rx_data, rx_len);
        inst->cur_recv_len += rx_len;

        if (inst->cur_recv_len == inst->recv_buf_len)
        {
            if (inst->raw_recvbuf[inst->recv_buf_len - 1] == CAN_COMM_TAIL)
            {
                if (inst->raw_recvbuf[inst->recv_buf_len - 2] == crc_8(inst->raw_recvbuf + 2, inst->recv_data_len))
                {
                    memcpy(inst->unpacked_recv_data, inst->raw_recvbuf + 2, inst->recv_data_len);
                    inst->update_flag = 1;
                }
            }
            CANCommResetRx(inst);
        }
    }
}

void *CANCommGet(CANCommInstance *inst)
{
    inst->update_flag = 0;
    return inst->unpacked_recv_data;
}
