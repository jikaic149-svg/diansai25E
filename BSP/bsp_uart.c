#include "bsp_uart.h"

/* ===== TX 使用 DMA，RX 使用 CPU 中断 ===== */

void BSP_UART_Init(void)
{
    /* === 蓝牙 UART0 DMA TX === */
    DL_DMA_setDestAddr(DMA, DMA_TX_Blue_CHAN_ID, (uint32_t) &UART_Blue_INST->TXDATA);

    /* === 循迹 UART2 DMA TX === */
    DL_DMA_setDestAddr(DMA, DMA_TX_Track_CHAN_ID, (uint32_t) &UART_Track_INST->TXDATA);

    /* ===========================================================
     * 彻底关闭 DMA RX 通道，改用 CPU 中断接收。
     * 注意：不能 disableFIFOs，因为 TX DMA 需要 TX FIFO！
     *   （MSPM0 的 FIFO 开关是 TX/RX 共用一个 bit）
     * 所以保留 FIFO，但在 ISR 里 while 循环排空 RX FIFO。
     * =========================================================== */

    /* --- WIT UART1 --- */
    DL_DMA_disableChannel(DMA, DMA_WIT_CHAN_ID);
    DL_UART_Main_disableDMAReceiveEvent(UART_WIT_INST, DL_UART_DMA_INTERRUPT_RX);
    DL_UART_Main_enableInterrupt(UART_WIT_INST,
        DL_UART_MAIN_INTERRUPT_RX | DL_UART_MAIN_INTERRUPT_RX_TIMEOUT_ERROR);

    /* --- 蓝牙 UART0 --- */
    DL_DMA_disableChannel(DMA, DMA_RX_Blue_CHAN_ID);
    DL_UART_Main_disableDMAReceiveEvent(UART_Blue_INST, DL_UART_DMA_INTERRUPT_RX);
    DL_UART_Main_enableInterrupt(UART_Blue_INST,
        DL_UART_MAIN_INTERRUPT_RX | DL_UART_MAIN_INTERRUPT_RX_TIMEOUT_ERROR);

    /* --- 循迹 UART2 --- */
    DL_DMA_disableChannel(DMA, DMA_RX_Track_CHAN_ID);
    DL_UART_Main_disableDMAReceiveEvent(UART_Track_INST, DL_UART_DMA_INTERRUPT_RX);
    DL_UART_Main_enableInterrupt(UART_Track_INST,
        DL_UART_MAIN_INTERRUPT_RX | DL_UART_MAIN_INTERRUPT_RX_TIMEOUT_ERROR);

    /* 使能 NVIC */
    NVIC_EnableIRQ(UART_WIT_INST_INT_IRQN);
    NVIC_EnableIRQ(UART_Blue_INST_INT_IRQN);
    NVIC_EnableIRQ(UART_Track_INST_INT_IRQN);
}

/* === DMA TX === */

void BSP_BT_SendPacket(uint8_t *data, uint16_t len)
{
    DL_DMA_disableChannel(DMA, DMA_TX_Blue_CHAN_ID);
    DL_DMA_setSrcAddr(DMA, DMA_TX_Blue_CHAN_ID, (uint32_t) data);
    DL_DMA_setTransferSize(DMA, DMA_TX_Blue_CHAN_ID, len);
    DL_DMA_enableChannel(DMA, DMA_TX_Blue_CHAN_ID);
}

void BSP_Track_SendPacket(uint8_t *data, uint16_t len)
{
    DL_DMA_disableChannel(DMA, DMA_TX_Track_CHAN_ID);
    DL_DMA_setSrcAddr(DMA, DMA_TX_Track_CHAN_ID, (uint32_t) data);
    DL_DMA_setTransferSize(DMA, DMA_TX_Track_CHAN_ID, len);
    DL_DMA_enableChannel(DMA, DMA_TX_Track_CHAN_ID);
}

/* ===================================================================
 * 通用 UART 接收处理：无论是 RX 还是 RX_TIMEOUT 触发，
 * 都 while 循环把 FIFO 里所有字节读完，逐个送入上层回调。
 * 这样就不会因为 FIFO 没排空导致中断卡死不再触发。
 * =================================================================== */

/* ========== WIT UART 中断处理（由 isr.c 调用） ========== */
void BSP_UART_WIT_IRQHandler(void)
{
    DL_UART_getPendingInterrupt(UART_WIT_INST);
    while (!DL_UART_Main_isRXFIFOEmpty(UART_WIT_INST)) {
        uint8_t c = DL_UART_Main_receiveData(UART_WIT_INST);
        BSP_WIT_ProcessPacket(&c, 1);
    }
}

/* ========== 蓝牙 UART 中断处理（由 isr.c 调用） ========== */
void BSP_UART_Blue_IRQHandler(void)
{
    DL_UART_getPendingInterrupt(UART_Blue_INST);
    while (!DL_UART_Main_isRXFIFOEmpty(UART_Blue_INST)) {
        uint8_t c = DL_UART_Main_receiveData(UART_Blue_INST);
        BSP_BT_ProcessPacket(&c, 1);
    }
}

/* ========== 循迹 UART 中断处理（由 isr.c 调用） ========== */
void BSP_UART_Track_IRQHandler(void)
{
    DL_UART_getPendingInterrupt(UART_Track_INST);
    while (!DL_UART_Main_isRXFIFOEmpty(UART_Track_INST)) {
        uint8_t c = DL_UART_Main_receiveData(UART_Track_INST);
        BSP_Track_ProcessPacket(&c, 1);
    }
}

/* 弱回调 */
__attribute__((weak)) void BSP_WIT_ProcessPacket(uint8_t *data, uint16_t len)
{ (void)data; (void)len; }

__attribute__((weak)) void BSP_BT_ProcessPacket(uint8_t *data, uint16_t len)
{ (void)data; (void)len; }

__attribute__((weak)) void BSP_Track_ProcessPacket(uint8_t *data, uint16_t len)
{ (void)data; (void)len; }
