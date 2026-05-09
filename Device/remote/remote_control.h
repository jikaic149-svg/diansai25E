#ifndef REMOTE_CONTROL_H
#define REMOTE_CONTROL_H

/**
 * @file remote_control.h
 * @brief 遥控器协议解析（支持DJI DT7 DBUS 和 SBUS FUS协议）
 * @note  已移除BSP依赖，通过函数指针抽象UART
 */

#include <stdint.h>

#define LAST 1
#define TEMP 0

#define KEY_PRESS 0
#define KEY_STATE 1
#define KEY_PRESS_WITH_CTRL 1
#define KEY_PRESS_WITH_SHIFT 2

#define RC_CH_VALUE_MIN ((uint16_t)364)
#define RC_CH_VALUE_OFFSET ((uint16_t)1024)
#define RC_CH_VALUE_MAX ((uint16_t)1684)

/* ----------------------- RC Switch Definition ----------------------------- */
#define RC_SW_UP ((uint16_t)1)
#define RC_SW_MID ((uint16_t)3)
#define RC_SW_DOWN ((uint16_t)2)
#define switch_is_down(s) (s == RC_SW_DOWN)
#define switch_is_mid(s) (s == RC_SW_MID)
#define switch_is_up(s) (s == RC_SW_UP)

/* ----------------------- PC Key Definition -------------------------------- */
#define Key_W 0
#define Key_S 1
#define Key_D 2
#define Key_A 3
#define Key_Shift 4
#define Key_Ctrl 5
#define Key_Q 6
#define Key_E 7
#define Key_R 8
#define Key_F 9
#define Key_G 10
#define Key_Z 11
#define Key_X 12
#define Key_C 13
#define Key_V 14
#define Key_B 15

/* ----------------------- Data Struct ------------------------------------- */
typedef union
{
    struct
    {
        uint16_t w : 1;
        uint16_t s : 1;
        uint16_t d : 1;
        uint16_t a : 1;
        uint16_t shift : 1;
        uint16_t ctrl : 1;
        uint16_t q : 1;
        uint16_t e : 1;
        uint16_t r : 1;
        uint16_t f : 1;
        uint16_t g : 1;
        uint16_t z : 1;
        uint16_t x : 1;
        uint16_t c : 1;
        uint16_t v : 1;
        uint16_t b : 1;
    };
    uint16_t keys;
} Key_t;

typedef struct
{
    struct
    {
        int16_t rocker_l_;  // 左水平
        int16_t rocker_l1;  // 左竖直
        int16_t rocker_r_;  // 右水平
        int16_t rocker_r1;  // 右竖直
        int16_t dial;       // 侧边拨轮

        uint8_t switch_left;
        uint8_t switch_right;
        uint8_t swa;
        uint8_t swb;
        uint8_t swc;
        uint8_t swd;
        int16_t dial1;
        int16_t dial2;
    } rc;
    struct
    {
        int16_t x;
        int16_t y;
        uint8_t press_l;
        uint8_t press_r;
    } mouse;

    Key_t key[3];
    uint8_t key_count[3][16];
} RC_ctrl_t;

/**
 * @brief DJI DT7 遥控器DBUS协议解析（18字节帧）
 * @param sbus_buf  接收到的18字节原始数据
 * @param rc_ctrl   遥控器数据结构体数组(大小为2, [0]=TEMP, [1]=LAST)
 */
void RC_DT7_DBUS_Decode(const uint8_t *sbus_buf, RC_ctrl_t *rc_ctrl);

/**
 * @brief SBUS FUS遥控器协议解析（25字节帧, 帧头0x0F）
 * @param sbus_buf  接收到的25字节原始数据
 * @param rc_ctrl   遥控器数据结构体数组(大小为2, [0]=TEMP, [1]=LAST)
 */
void RC_SBUS_FUS_Decode(const uint8_t *sbus_buf, RC_ctrl_t *rc_ctrl);

#endif
