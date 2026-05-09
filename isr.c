/**
 * @file isr.c
 * @brief 系统中断服务例程集中管理（Rule 5: ISR 物理隔离）
 *        所有硬件 ISR 入口汇聚于此，仅做标记/转发，严禁耗时业务逻辑
 */
#include "ti_msp_dl_config.h"
#include "BSP/bsp_timer.h"
#include "BSP/bsp_uart.h"
#include "BSP/bsp_encoder.h"

/* ===== SysTick：系统时间戳 (1ms) ===== */
void SysTick_Handler(void)
{
    BSP_Timer_SysTick_Handler();
}

/* ===== TIMG0：Schedule 调度 tick ===== */
void SCHEDULE_TIMER_INST_IRQHandler(void)
{
    BSP_Timer_Schedule_IRQHandler();
}

/* ===== TIMG12：QEI 采样定时器 (1ms) ===== */
void QEI_SAMPLE_TIMER_INST_IRQHandler(void)
{
    BSP_Timer_QEI_IRQHandler();
}

/* ===== UART 中断 (仅留驱动层解析抛出) ===== */
void UART_WIT_INST_IRQHandler(void)
{
    BSP_UART_WIT_IRQHandler();
}

void UART_Blue_INST_IRQHandler(void)
{
    BSP_UART_Blue_IRQHandler();
}

void UART_Track_INST_IRQHandler(void)
{
    BSP_UART_Track_IRQHandler();
}

/* ===== GPIOB：编码器正交解码 ===== */
void GROUP1_IRQHandler(void)
{
    BSP_Encoder_GPIOB_IRQHandler();
}
