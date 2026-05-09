/**
 * @file super_cap.c
 * @brief 超级电容模块实现
 */

#include "super_cap.h"
#include <string.h>

void SuperCapInit(SuperCapInstance *inst, uint32_t tx_id, SuperCap_CANSend_t send_fn)
{
    memset(inst, 0, sizeof(SuperCapInstance));
    inst->tx_id = tx_id;
    inst->can_send = send_fn;
}

void SuperCapDecode(SuperCapInstance *inst, const uint8_t *rxbuff)
{
    inst->cap_msg.vol     = (uint16_t)(rxbuff[0] << 8 | rxbuff[1]);
    inst->cap_msg.current = (uint16_t)(rxbuff[2] << 8 | rxbuff[3]);
    inst->cap_msg.power   = (uint16_t)(rxbuff[4] << 8 | rxbuff[5]);
}

void SuperCapSend(SuperCapInstance *inst, const uint8_t *data)
{
    if (inst->can_send)
        inst->can_send(inst->tx_id, data);
}

SuperCap_Msg_s SuperCapGet(SuperCapInstance *inst)
{
    return inst->cap_msg;
}
