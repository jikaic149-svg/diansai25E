/*
 * main.c — 系统入口，所有逻辑由 schedule 模块管理
 */
#include "schedule.h"

int main(void)
{
    Schedule_Init();

    while (1) {
        Schedule_Run();
    }
}
