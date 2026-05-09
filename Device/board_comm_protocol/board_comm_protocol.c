/**
 * @file    board_comm_protocol.c
 * @brief   板间通信协议实现
 * @origin  底盘 board_comm.c + 云台 timachine/board_comm.c
 * @note    通过函数指针解耦UART驱动, 100%可移植
 */
#include "board_comm_protocol.h"
#include <string.h>

static BoardCommData_t s_data = {0};
static board_comm_send_byte_func_t s_send_byte = 0;

/* 计算简单校验和 */
static uint32_t calc_checksum(const uint8_t *data, uint8_t len)
{
    uint32_t sum = 0;
    for (uint8_t i = 0; i < len; i++)
        sum += data[i];
    return sum;
}

void BoardComm_Init(board_comm_send_byte_func_t send_byte_func)
{
    s_send_byte = send_byte_func;
    memset(&s_data, 0, sizeof(s_data));
}

void BoardComm_Send(int32_t int_data[2], float float_data[2])
{
    if (!s_send_byte) return;

    BoardCommPacket_t packet;
    packet.header[0] = BOARD_COMM_HEADER_0;
    packet.header[1] = BOARD_COMM_HEADER_1;

    memcpy(packet.int_data, int_data, sizeof(packet.int_data));
    memcpy(packet.float_data, float_data, sizeof(packet.float_data));
    memcpy(s_data.tx_int_data, int_data, sizeof(s_data.tx_int_data));
    memcpy(s_data.tx_float_data, float_data, sizeof(s_data.tx_float_data));

    uint8_t data_len = sizeof(packet.int_data) + sizeof(packet.float_data);
    packet.checksum = calc_checksum((const uint8_t *)packet.int_data, data_len);

    const uint8_t *raw = (const uint8_t *)&packet;
    for (uint8_t i = 0; i < sizeof(packet); i++)
        s_send_byte(raw[i]);
}

uint8_t BoardComm_Decode(const uint8_t *buffer)
{
    const BoardCommPacket_t *packet = (const BoardCommPacket_t *)buffer;

    // 验证帧头
    if (packet->header[0] != BOARD_COMM_HEADER_0 || packet->header[1] != BOARD_COMM_HEADER_1)
        return 0;

    // 验证校验和
    uint8_t data_len = sizeof(packet->int_data) + sizeof(packet->float_data);
    uint32_t expected = calc_checksum((const uint8_t *)packet->int_data, data_len);
    if (expected != packet->checksum)
        return 0;

    memcpy(s_data.rx_int_data, packet->int_data, sizeof(packet->int_data));
    memcpy(s_data.rx_float_data, packet->float_data, sizeof(packet->float_data));
    return 1;
}

int32_t *BoardComm_GetIntData(void)   { return s_data.rx_int_data; }
float   *BoardComm_GetFloatData(void) { return s_data.rx_float_data; }
