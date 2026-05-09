/**
 * @file remote_control.c
 * @brief 遥控器协议解析（纯协议层，无BSP依赖）
 */

#include "remote_control.h"
#include <string.h>
#include <stdlib.h>

/* =================== DJI DT7 DBUS 协议 =================== */

static void RectifyRCjoystick_DT7(RC_ctrl_t *rc)
{
    for (uint8_t i = 0; i < 5; ++i)
        if (abs(*(&rc[TEMP].rc.rocker_l_ + i)) > 660)
            *(&rc[TEMP].rc.rocker_l_ + i) = 0;
}

void RC_DT7_DBUS_Decode(const uint8_t *sbus_buf, RC_ctrl_t *rc_ctrl)
{
    // 摇杆
    rc_ctrl[TEMP].rc.rocker_r_ = ((sbus_buf[0] | (sbus_buf[1] << 8)) & 0x07ff) - RC_CH_VALUE_OFFSET;
    rc_ctrl[TEMP].rc.rocker_r1 = (((sbus_buf[1] >> 3) | (sbus_buf[2] << 5)) & 0x07ff) - RC_CH_VALUE_OFFSET;
    rc_ctrl[TEMP].rc.rocker_l_ = (((sbus_buf[2] >> 6) | (sbus_buf[3] << 2) | (sbus_buf[4] << 10)) & 0x07ff) - RC_CH_VALUE_OFFSET;
    rc_ctrl[TEMP].rc.rocker_l1 = (((sbus_buf[4] >> 1) | (sbus_buf[5] << 7)) & 0x07ff) - RC_CH_VALUE_OFFSET;
    rc_ctrl[TEMP].rc.dial = ((sbus_buf[16] | (sbus_buf[17] << 8)) & 0x07FF) - RC_CH_VALUE_OFFSET;
    RectifyRCjoystick_DT7(rc_ctrl);

    // 开关
    rc_ctrl[TEMP].rc.switch_right = ((sbus_buf[5] >> 4) & 0x0003);
    rc_ctrl[TEMP].rc.switch_left = ((sbus_buf[5] >> 4) & 0x000C) >> 2;

    // 鼠标
    rc_ctrl[TEMP].mouse.x = (sbus_buf[6] | (sbus_buf[7] << 8));
    rc_ctrl[TEMP].mouse.y = (sbus_buf[8] | (sbus_buf[9] << 8));
    rc_ctrl[TEMP].mouse.press_l = sbus_buf[12];
    rc_ctrl[TEMP].mouse.press_r = sbus_buf[13];

    // 键盘
    *(uint16_t *)&rc_ctrl[TEMP].key[KEY_PRESS] = (uint16_t)(sbus_buf[14] | (sbus_buf[15] << 8));
    if (rc_ctrl[TEMP].key[KEY_PRESS].ctrl)
        rc_ctrl[TEMP].key[KEY_PRESS_WITH_CTRL] = rc_ctrl[TEMP].key[KEY_PRESS];
    else
        memset(&rc_ctrl[TEMP].key[KEY_PRESS_WITH_CTRL], 0, sizeof(Key_t));
    if (rc_ctrl[TEMP].key[KEY_PRESS].shift)
        rc_ctrl[TEMP].key[KEY_PRESS_WITH_SHIFT] = rc_ctrl[TEMP].key[KEY_PRESS];
    else
        memset(&rc_ctrl[TEMP].key[KEY_PRESS_WITH_SHIFT], 0, sizeof(Key_t));

    uint16_t key_now = rc_ctrl[TEMP].key[KEY_PRESS].keys,
             key_last = rc_ctrl[LAST].key[KEY_PRESS].keys,
             key_with_ctrl = rc_ctrl[TEMP].key[KEY_PRESS_WITH_CTRL].keys,
             key_with_shift = rc_ctrl[TEMP].key[KEY_PRESS_WITH_SHIFT].keys,
             key_last_with_ctrl = rc_ctrl[LAST].key[KEY_PRESS_WITH_CTRL].keys,
             key_last_with_shift = rc_ctrl[LAST].key[KEY_PRESS_WITH_SHIFT].keys;

    for (uint16_t i = 0, j = 0x1; i < 16; j <<= 1, i++)
    {
        if (i == 4 || i == 5) continue;
        if ((key_now & j) && !(key_last & j) && !(key_with_ctrl & j) && !(key_with_shift & j))
            rc_ctrl[TEMP].key_count[KEY_PRESS][i]++;
        if ((key_with_ctrl & j) && !(key_last_with_ctrl & j))
            rc_ctrl[TEMP].key_count[KEY_PRESS_WITH_CTRL][i]++;
        if ((key_with_shift & j) && !(key_last_with_shift & j))
            rc_ctrl[TEMP].key_count[KEY_PRESS_WITH_SHIFT][i]++;
    }

    memcpy(&rc_ctrl[LAST], &rc_ctrl[TEMP], sizeof(RC_ctrl_t));
}

/* =================== SBUS FUS 协议 =================== */

static void RectifyRCjoystick_SBUS(RC_ctrl_t *rc)
{
    for (uint8_t i = 0; i < 5; ++i)
        if (abs(*(&rc[TEMP].rc.rocker_l_ + i)) > 785)
            *(&rc[TEMP].rc.rocker_l_ + i) = 0;
}

void RC_SBUS_FUS_Decode(const uint8_t *sbus_buf, RC_ctrl_t *rc_ctrl)
{
    static int16_t sRecvChanelData[16];

    sRecvChanelData[0] = ((sbus_buf[2] & 0x07) << 8) + ((sbus_buf[1] >> 0) & 0xFF);
    sRecvChanelData[1] = ((sbus_buf[3] & 0x3F) << 5) + ((sbus_buf[2] >> 3) & 0x1F);
    sRecvChanelData[2] = ((sbus_buf[5] & 0x01) << 10) + ((sbus_buf[4] & 0xFF) << 2) + ((sbus_buf[3] >> 6) & 0x03);
    sRecvChanelData[3] = ((sbus_buf[6] & 0x0F) << 7) + ((sbus_buf[5] >> 1) & 0x7F);
    sRecvChanelData[4] = ((sbus_buf[7] & 0x7F) << 4) + ((sbus_buf[6] >> 4) & 0x0F);
    sRecvChanelData[5] = ((sbus_buf[9] & 0x03) << 9) + ((sbus_buf[8] & 0xFF) << 1) + ((sbus_buf[7] >> 7) & 0x01);
    sRecvChanelData[6] = ((sbus_buf[10] & 0x1F) << 6) + ((sbus_buf[9] >> 2) & 0x3F);
    sRecvChanelData[7] = ((sbus_buf[11] & 0xFF) << 3) + ((sbus_buf[10] >> 5) & 0x07);
    sRecvChanelData[8] = ((sbus_buf[13] & 0x07) << 8) + ((sbus_buf[12] >> 0) & 0xFF);
    sRecvChanelData[9] = ((sbus_buf[14] & 0x3F) << 5) + ((sbus_buf[13] >> 3) & 0x1F);

    rc_ctrl[TEMP].rc.rocker_r_ = sRecvChanelData[0] - RC_CH_VALUE_OFFSET;
    rc_ctrl[TEMP].rc.rocker_r1 = sRecvChanelData[1] - RC_CH_VALUE_OFFSET;
    rc_ctrl[TEMP].rc.rocker_l1 = sRecvChanelData[2] - RC_CH_VALUE_OFFSET;
    rc_ctrl[TEMP].rc.rocker_l_ = sRecvChanelData[3] - RC_CH_VALUE_OFFSET;
    rc_ctrl[TEMP].rc.dial1 = sRecvChanelData[4] - RC_CH_VALUE_OFFSET;
    rc_ctrl[TEMP].rc.dial2 = sRecvChanelData[5] - RC_CH_VALUE_OFFSET;

    rc_ctrl[TEMP].rc.swa = (sRecvChanelData[6] == 240) ? RC_SW_UP : RC_SW_DOWN;
    rc_ctrl[TEMP].rc.swb = (sRecvChanelData[7] == 240) ? RC_SW_UP : RC_SW_DOWN;

    if (sRecvChanelData[8] == 240)
        rc_ctrl[TEMP].rc.swc = RC_SW_UP;
    else if (sRecvChanelData[8] == 1024)
        rc_ctrl[TEMP].rc.swc = RC_SW_MID;
    else
        rc_ctrl[TEMP].rc.swc = RC_SW_DOWN;

    rc_ctrl[TEMP].rc.swd = (sRecvChanelData[9] == 240) ? RC_SW_UP : RC_SW_DOWN;

    RectifyRCjoystick_SBUS(rc_ctrl);
}
