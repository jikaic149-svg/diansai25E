/**
 * @file    display_menu.h
 * @brief   通用参数菜单系统（屏幕显示 + 按键编辑模式）
 * @origin  底盘 project/code/display.h + display.c
 * @note    与具体屏幕驱动解耦, 用户需实现 display_port_t 中的函数指针
 *
 * 使用方法:
 *   1. 实现 display_port_t 中的屏幕回调
 *   2. 调用 Menu_Init() 注册参数列表
 *   3. 在按键处理中调用 Menu_KeyXxx()
 *   4. 在主循环中调用 Menu_Draw() 刷新界面
 */
#ifndef DISPLAY_MENU_H
#define DISPLAY_MENU_H

#include <stdint.h>

/* 参数项定义 */
typedef struct
{
    char     name[20];      // 参数名
    void    *value;         // 参数值指针(int* 或 float*)
    float    step;          // 调整步长
    float    min_val;       // 最小值
    float    max_val;       // 最大值
    uint8_t  is_int;        // 1=整数参数, 0=浮点参数
} ParamItem_t;

/* 屏幕驱动接口(用户实现) */
typedef struct
{
    void (*show_string)(uint16_t x, uint16_t y, const char *str);
    void (*show_int)(uint16_t x, uint16_t y, int value, uint8_t digits);
    void (*show_float)(uint16_t x, uint16_t y, float value, uint8_t int_digits, uint8_t dec_digits);
    void (*clear_screen)(void);
} display_port_t;

/**
 * @brief 初始化菜单系统
 * @param params       参数数组
 * @param param_count  参数数量
 * @param port         屏幕驱动接口
 */
void Menu_Init(ParamItem_t *params, uint8_t param_count, display_port_t *port);

/**
 * @brief 绘制参数编辑页面
 */
void Menu_Draw(void);

/**
 * @brief 是否处于编辑模式
 */
uint8_t Menu_IsEditMode(void);

/**
 * @brief 进入编辑模式
 */
void Menu_EnterEdit(void);

/**
 * @brief 退出编辑模式
 */
void Menu_ExitEdit(void);

/**
 * @brief 选择上一个参数
 */
void Menu_SelectPrev(void);

/**
 * @brief 选择下一个参数
 */
void Menu_SelectNext(void);

/**
 * @brief 增加当前参数值
 */
void Menu_ValueIncrease(void);

/**
 * @brief 减少当前参数值
 */
void Menu_ValueDecrease(void);

#endif /* DISPLAY_MENU_H */
