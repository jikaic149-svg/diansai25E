/**
 * @file track.c
 * @brief 8路红外模拟循迹模块驱动实现
 */
#include "track.h"
#include <string.h>
#include <stdlib.h>

void Track_Init(Track_Instance *inst, Track_UartSend_t send_fn)
{
    inst->uart_send = send_fn;
    inst->data_ready = 0;
    memset(inst->adc_value, 0, sizeof(inst->adc_value));

    /* 发送启动命令 "$0,1,0#" 让循迹模块开始输出数据 */
    /* 必须用 static，否则 DMA 还没传完数据函数就返回了，栈变量会被覆盖！ */
    if (inst->uart_send) {
        static const uint8_t cmd[] = "$0,1,0#";
        inst->uart_send((uint8_t *)cmd, sizeof(cmd) - 1);
    }
}

void Track_ParsePacket(Track_Instance *inst, const uint8_t *data, uint16_t len)
{
    /*
     * 协议格式: "$A,x1:val,x2:val,x3:val,x4:val,x5:val,x6:val,x7:val,x8:val#"
     * 用手动解析代替 sscanf 以节省代码空间和提升速度
     */
    if (len < 5) return;

    /* 查找帧头 '$A,' */
    const uint8_t *p = data;
    const uint8_t *end = data + len;

    while (p < end - 4) {
        if (p[0] == '$' && p[1] == 'A' && p[2] == ',') {
            p += 3;  /* 跳过 "$A," */
            break;
        }
        p++;
    }
    if (p >= end - 4) return;

    /* 逐个解析 "xN:val," */
    uint8_t ch = 0;
    while (p < end && ch < TRACK_SENSOR_COUNT) {
        /* 跳过 "xN:" 前缀 */
        if (*p == 'x') {
            p++;  /* 跳过 'x' */
            if (p < end) p++;  /* 跳过通道数字 */
            if (p < end && *p == ':') p++;  /* 跳过 ':' */
        }

        /* 解析数值 */
        uint16_t val = 0;
        while (p < end && *p >= '0' && *p <= '9') {
            val = val * 10 + (*p - '0');
            p++;
        }
        inst->adc_value[ch] = val;
        ch++;

        /* 跳过分隔符 ',' 或终止符 '#' */
        if (p < end && (*p == ',' || *p == '#')) p++;
    }

    if (ch > 0) {
        inst->data_ready = 1;
    }
}

float Track_GetError(Track_Instance *inst)
{
    /* 探头位置权重 (左正右负) — 削弱边缘权重，让直线纠偏更平滑 */
    const float weights[8] = {3.5f, 2.5f, 1.5f, 0.5f, -0.5f, -1.5f, -2.5f, -3.5f};

    /* 纯白底基准值（根据 1.txt 最新标定数据均值） */
    const uint16_t WHITE_BASE[8] = {239, 193, 192, 195, 400, 192, 192, 238};

    /* 触发阈值：白底噪声通常在 +-20。降到 80 能提取出边缘部分遮挡的灰度值，
     * 极大地平滑加权求和（质心算法）的输出，防止差速在 0.5 和 -1.5 之间跳变引起车身震颤 */
    const float DIFF_THRESHOLD = 80.0f;

    float weighted_sum = 0.0f;
    float total_black  = 0.0f;

    for (int i = 0; i < 8; i++) {
        float diff = 0.0f;
        if (inst->adc_value[i] > WHITE_BASE[i]) {
            diff = (float)(inst->adc_value[i] - WHITE_BASE[i]);
        }

        if (diff > DIFF_THRESHOLD) {
            weighted_sum += diff * weights[i];
            total_black  += diff;
        }
    }

    if (total_black == 0.0f) return TRACK_LOST_LINE;

    return (weighted_sum / total_black);
}
