/**
 * @file opencv.c
 * @brief OpenCV上位机视觉通信模块实现
 */

#include "opencv.h"
#include <string.h>

float opencv_Vision_data[12];
uint8_t Vison_revc_flag = 0;

void Vision_DecodeFrame(const uint8_t *buf, uint16_t len)
{
    if (len != OPENCV_FRAME_FULL_LEN)
        return;

    if (buf[0] != OPENCV_FRAME_SOF)
        return;

    uint8_t payload_len = buf[1];
    if (payload_len != OPENCV_FRAME_PAYLOAD_LEN)
        return;

    // cmd_id, flags 可按需解析
    // uint16_t cmd_id = buf[2] | (buf[3] << 8);
    // uint16_t flags  = buf[4] | (buf[5] << 8);

    for (int i = 0; i < 12; i++)
        memcpy(&opencv_Vision_data[i], buf + 6 + 4 * i, sizeof(float));

    Vison_revc_flag = 1;
}
