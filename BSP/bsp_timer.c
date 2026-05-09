/**
 * @file bsp_timer.c
 * @brief BSP 定时器实现
 *        SysTick: 系统时间戳 (蓝牙输出 [秒.毫秒])
 *        TIMG0:   Schedule 调度器 tick 源
 */
#include "bsp_timer.h"
#include "ti_msp_dl_config.h"

/* ===== 系统级初始化 ===== */

void BSP_System_Init(void)
{
    SYSCFG_DL_init();
    delay_cycles(160000);
}

void BSP_System_EnableIRQ(void)
{
    __enable_irq();
}

/* SysTick：系统时间戳 */
static volatile uint32_t g_tick_ms_32 = 0;

/* TIMG0：调度器 tick（独立计数） */
static volatile uint16_t g_schedule_tick = 0;

/* ===== SysTick ===== */

void BSP_Timer_Init(void)
{
    SysTick_Config(32000);  /* 1ms */
}

uint32_t BSP_Timer_GetTickMs(void)
{
    return g_tick_ms_32;
}

void BSP_Timer_SysTick_Handler(void)
{
    g_tick_ms_32++;
}

/* ===== TIMG0: Schedule 调度 tick ===== */

void BSP_Timer_ScheduleInit(void)
{
    NVIC_EnableIRQ(SCHEDULE_TIMER_INST_INT_IRQN);
    DL_TimerG_startCounter(SCHEDULE_TIMER_INST);
}

uint16_t BSP_Timer_GetScheduleTick(void)
{
    return g_schedule_tick;
}

void BSP_Timer_Schedule_IRQHandler(void)
{
    DL_TimerG_clearInterruptStatus(SCHEDULE_TIMER_INST, DL_TIMER_INTERRUPT_ZERO_EVENT);
    g_schedule_tick++;
}

/* ===== TIMG12: QEI 采样定时器 ===== */

static volatile bool    s_enc_update_flag = false;
static volatile uint8_t s_qei_ms_cnt      = 0;

void BSP_Timer_QEI_Start(void)
{
    NVIC_EnableIRQ(QEI_SAMPLE_TIMER_INST_INT_IRQN);
    DL_TimerG_startCounter(QEI_SAMPLE_TIMER_INST);
}

void BSP_Timer_QEI_IRQHandler(void)
{
    DL_TimerG_clearInterruptStatus(QEI_SAMPLE_TIMER_INST, DL_TIMER_INTERRUPT_ZERO_EVENT);
    if (++s_qei_ms_cnt >= 10) {
        s_qei_ms_cnt = 0;
        s_enc_update_flag = true;
    }
}

bool BSP_Timer_IsEncUpdateReady(void)
{
    return s_enc_update_flag;
}

void BSP_Timer_ClearEncUpdateFlag(void)
{
    s_enc_update_flag = false;
}
