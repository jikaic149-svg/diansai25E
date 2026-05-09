/**
 * @file    display_menu.c
 * @brief   通用参数菜单系统实现
 * @origin  底盘 project/code/display.c
 * @note    通过 display_port_t 函数指针解耦屏幕驱动, 可用于任何屏幕
 */
#include "display_menu.h"
#include <string.h>

static ParamItem_t   *s_params = 0;
static uint8_t        s_param_count = 0;
static display_port_t s_port = {0};
static uint8_t        s_selected = 0;
static uint8_t        s_edit_mode = 0;

void Menu_Init(ParamItem_t *params, uint8_t param_count, display_port_t *port)
{
    s_params = params;
    s_param_count = param_count;
    if (port) memcpy(&s_port, port, sizeof(display_port_t));
    s_selected = 0;
    s_edit_mode = 0;
}

void Menu_Draw(void)
{
    if (!s_params || !s_port.show_string) return;

    s_port.show_string(0, 0, s_edit_mode ? "PARAM EDIT MODE" : "PARAM VIEW");
    s_port.show_string(0, 16, "UP DN + -");

    for (uint8_t i = 0; i < s_param_count; i++)
    {
        uint16_t y = 48 + i * 16;
        if (y > 300) break;

        s_port.show_string(0, y, (i == s_selected) ? ">" : " ");
        s_port.show_string(16, y, s_params[i].name);

        if (s_params[i].is_int && s_port.show_int)
        {
            int *ip = (int *)s_params[i].value;
            s_port.show_int(140, y, *ip, 5);
        }
        else if (!s_params[i].is_int && s_port.show_float)
        {
            float *fp = (float *)s_params[i].value;
            s_port.show_float(140, y, *fp, 3, 2);
        }
    }
}

uint8_t Menu_IsEditMode(void)    { return s_edit_mode; }
void    Menu_EnterEdit(void)     { s_edit_mode = 1; s_selected = 0; }
void    Menu_ExitEdit(void)      { s_edit_mode = 0; }

void Menu_SelectPrev(void)
{
    if (!s_edit_mode) return;
    s_selected = (s_selected > 0) ? (s_selected - 1) : (s_param_count - 1);
}

void Menu_SelectNext(void)
{
    if (!s_edit_mode) return;
    s_selected = (s_selected < s_param_count - 1) ? (s_selected + 1) : 0;
}

void Menu_ValueIncrease(void)
{
    if (!s_edit_mode || !s_params) return;
    ParamItem_t *p = &s_params[s_selected];
    if (p->is_int)
    {
        int *ip = (int *)p->value;
        int nv = *ip + (int)p->step;
        if (nv <= (int)p->max_val) *ip = nv;
    }
    else
    {
        float *fp = (float *)p->value;
        float nv = *fp + p->step;
        if (nv <= p->max_val) *fp = nv;
    }
}

void Menu_ValueDecrease(void)
{
    if (!s_edit_mode || !s_params) return;
    ParamItem_t *p = &s_params[s_selected];
    if (p->is_int)
    {
        int *ip = (int *)p->value;
        int nv = *ip - (int)p->step;
        if (nv >= (int)p->min_val) *ip = nv;
    }
    else
    {
        float *fp = (float *)p->value;
        float nv = *fp - p->step;
        if (nv >= p->min_val) *fp = nv;
    }
}
