#ifndef OPENCV_H
#define OPENCV_H

/**
 * @file opencv.h
 * @brief OpenCV 上位机视觉通信模块（UART抽象）
 * @note  协议帧: [SOF 0xA5] [payload_len] [cmd_id_L cmd_id_H] [flags_L flags_H] [12 floats]
 *        总长54字节
 */

#include <stdint.h>

#define OPENCV_FRAME_SOF        0xA5
#define OPENCV_FRAME_PAYLOAD_LEN (2 + 2 + 4 * 12) // 52
#define OPENCV_FRAME_FULL_LEN   (1 + 1 + OPENCV_FRAME_PAYLOAD_LEN) // 54

extern float opencv_Vision_data[12];
extern uint8_t Vison_revc_flag;

/**
 * @brief 解析接收到的视觉帧数据(54字节)
 * @param buf  接收缓冲区(54字节)
 * @param len  缓冲区长度
 */
void Vision_DecodeFrame(const uint8_t *buf, uint16_t len);

#endif
