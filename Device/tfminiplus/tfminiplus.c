/**
 * @file tfminiplus.c
 * @brief TFminiPlus激光雷达测距模块实现
 */

#include "tfminiplus.h"

void TFMiniInit(TFMiniInstance *inst, uint8_t dev_addr,
                TFMini_I2CSend_t send_fn, TFMini_I2CRecv_t recv_fn)
{
    inst->dev_addr = dev_addr;
    inst->i2c_send = send_fn;
    inst->i2c_recv = recv_fn;
    inst->Dist = 0;
    inst->Strength = 0;
    inst->Mode = 0;
}

float TFMiniGetDistance(TFMiniInstance *inst)
{
    uint8_t cmd_buf[5] = {0x5A, 0x05, 0x00, 0x06, 0x65};
    inst->i2c_send(inst->dev_addr, cmd_buf, 5);
    inst->i2c_recv(inst->dev_addr, inst->buf, 11);

    inst->Mode = inst->buf[6];
    inst->Dist = (uint16_t)inst->buf[2] + (((uint16_t)inst->buf[3]) << 8);
    inst->Strength = inst->buf[4] | (inst->buf[5] << 8);

    return (float)inst->Dist;
}
