/**
 * @file vofa.c
 * @brief VOFA+ JustFloat 调试协议实现
 */

#include "vofa.h"
#include <string.h>

typedef union
{
    float float_t;
    uint8_t uint8_t[4];
} send_float;

static VOFA_UartSend_t vofa_send_fn = (void *)0;

void VOFA_RegisterSend(VOFA_UartSend_t send_fn)
{
    vofa_send_fn = send_fn;
}

void vofa_justfloat_output(float *data, uint8_t num)
{
    if (!vofa_send_fn)
        return;

    uint8_t i;
    send_float temp[16]; // 最大支持16个float
    if (num > 16) num = 16;

    /* 必须使用 static！因为底层的 UART 采用了 DMA 异步发送，如果分配在普通栈区，本函数一返回
       由于外设读取慢，DMA 会一直在后台读取已被释放/覆盖的脏内存空间，导致 VOFA 收到满屏乱码甚至单片机死机。 */
    static uint8_t send_data[4 * 16 + 4];
    for (i = 0; i < num; i++)
        temp[i].float_t = data[i];

    for (i = 0; i < num; i++)
    {
        send_data[4 * i]     = temp[i].uint8_t[0];
        send_data[4 * i + 1] = temp[i].uint8_t[1];
        send_data[4 * i + 2] = temp[i].uint8_t[2];
        send_data[4 * i + 3] = temp[i].uint8_t[3];
    }
    // 协议尾部
    send_data[4 * num]     = 0x00;
    send_data[4 * num + 1] = 0x00;
    send_data[4 * num + 2] = 0x80;
    send_data[4 * num + 3] = 0x7f;

    vofa_send_fn(send_data, 4 * num + 4);
}
