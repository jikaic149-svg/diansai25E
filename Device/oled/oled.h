#ifndef OLED_H
#define OLED_H

#include <stdint.h>

#define OLED_I2C_ADDRESS 0x78
#define MAX_COLUMN 128
#define MAX_ROW 64
#define X_WIDTH MAX_COLUMN
#define Y_WIDTH MAX_ROW
#define OLED_CMD 0x00
#define OLED_DATA 0x01
#define CHAR_SIZE_WIDTH 6
#define CHAR_SIZE_HIGHT 12

typedef enum
{
  PEN_CLEAR = 0x00,
  PEN_WRITE = 0x01,
  PEN_INVERSION = 0x02,
} pen_typedef;

/* =================== 硬件抽象接口 =================== */

/**
 * @brief I2C发送函数类型(用户需实现并注册)
 * @param addr  I2C设备地址
 * @param data  待发送数据
 * @param len   数据长度
 * @param timeout 超时(ms)
 */
typedef void (*OLED_I2CSend_t)(uint8_t addr, uint8_t *data, uint16_t len, uint16_t timeout);

/**
 * @brief 注册I2C发送函数
 */
void OLED_RegisterI2C(OLED_I2CSend_t send_fn);

/* =================== OLED操作接口 =================== */

void OLED_init(void);
void OLED_display_on(void);
void OLED_display_off(void);
void OLED_operate_gram(pen_typedef pen);
void OLED_set_pos(uint8_t x, uint8_t y);
void OLED_draw_point(int8_t x, int8_t y, pen_typedef pen);
void OLED_draw_line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, pen_typedef pen);
void OLED_show_char(uint8_t row, uint8_t col, uint8_t chr);
void OLED_show_string(uint8_t row, uint8_t col, uint8_t *chr);
void OLED_printf(uint8_t row, uint8_t col, const char *fmt, ...);
void OLED_refresh_gram(void);
void OLED_LOGO(void);

#endif
