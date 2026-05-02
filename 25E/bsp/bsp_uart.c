/**
 * @file    bsp_uart.c
 * @brief   BSP层 — USART1 底层操作 (STM32F407)
 * @note    DMA TX: DMA2_Stream7_Channel4
 *          DMA RX: DMA2_Stream5_Channel4
 *          IDLE 中断接收整帧
 *          PA9 = TX (AF7), PA10 = RX (AF7)
 *          波特率: 115200, 8N1
 */

#include "bsp_uart.h"
#include "stm32f4xx_hal.h"
#include <string.h>

/* ======================== 硬件句柄 ======================== */
static UART_HandleTypeDef huart1;
static DMA_HandleTypeDef  hdma_usart1_tx;
static DMA_HandleTypeDef  hdma_usart1_rx;

/* ======================== 缓冲区 ======================== */
static uint8_t uart1_rx_buf[BSP_UART1_RX_BUF_SIZE];
static volatile uint8_t uart1_tx_busy = 0;

/* ======================== 回调 ======================== */
static BSP_UART1_RxCallback_t rx_callback = NULL;

/* ======================== 初始化 ======================== */

void BSP_UART1_Init(void)
{
    /* ----- 1. 时钟使能 ----- */
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_DMA2_CLK_ENABLE();

    /* ----- 2. GPIO: PA9(TX), PA10(RX) ----- */
    GPIO_InitTypeDef gpio = {0};
    gpio.Pin       = GPIO_PIN_9 | GPIO_PIN_10; // PA9 TX, PA10 RX
    gpio.Mode      = GPIO_MODE_AF_PP;
    gpio.Pull      = GPIO_PULLUP;
    gpio.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &gpio);

    /* ----- 3. USART1 配置 ----- */
    huart1.Instance          = USART1;
    huart1.Init.BaudRate     = 115200;
    huart1.Init.WordLength   = UART_WORDLENGTH_8B;
    huart1.Init.StopBits     = UART_STOPBITS_1;
    huart1.Init.Parity       = UART_PARITY_NONE;
    huart1.Init.Mode         = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart1);

    /* ----- 4. DMA TX: DMA2_Stream7_Channel4 ----- */
    hdma_usart1_tx.Instance                 = DMA2_Stream7;
    hdma_usart1_tx.Init.Channel             = DMA_CHANNEL_4;
    hdma_usart1_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    hdma_usart1_tx.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_usart1_tx.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_usart1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart1_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    hdma_usart1_tx.Init.Mode                = DMA_NORMAL;
    hdma_usart1_tx.Init.Priority            = DMA_PRIORITY_MEDIUM;
    hdma_usart1_tx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    HAL_DMA_Init(&hdma_usart1_tx);
    __HAL_LINKDMA(&huart1, hdmatx, hdma_usart1_tx);

    /* ----- 5. DMA RX: DMA2_Stream5_Channel4 ----- */
    hdma_usart1_rx.Instance                 = DMA2_Stream5;
    hdma_usart1_rx.Init.Channel             = DMA_CHANNEL_4;
    hdma_usart1_rx.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    hdma_usart1_rx.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_usart1_rx.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_usart1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart1_rx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    hdma_usart1_rx.Init.Mode                = DMA_CIRCULAR;
    hdma_usart1_rx.Init.Priority            = DMA_PRIORITY_HIGH;
    hdma_usart1_rx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    HAL_DMA_Init(&hdma_usart1_rx);
    __HAL_LINKDMA(&huart1, hdmarx, hdma_usart1_rx);

    /* ----- 6. NVIC ----- */
    HAL_NVIC_SetPriority(USART1_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);

    HAL_NVIC_SetPriority(DMA2_Stream7_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream7_IRQn);

    HAL_NVIC_SetPriority(DMA2_Stream5_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream5_IRQn);

    /* ----- 7. 启动 IDLE 中断 + DMA 接收 ----- */
    __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);
    HAL_UART_Receive_DMA(&huart1, uart1_rx_buf, BSP_UART1_RX_BUF_SIZE);
}

/* ======================== 注册回调 ======================== */

void BSP_UART1_RegisterRxCallback(BSP_UART1_RxCallback_t callback)
{
    rx_callback = callback;
}

/* ======================== DMA 发送 ======================== */

bool BSP_UART1_SendDMA(const uint8_t *data, uint16_t len)
{
    if (uart1_tx_busy || len == 0 || len > BSP_UART1_TX_BUF_SIZE)
        return false;

    uart1_tx_busy = 1;
    HAL_UART_Transmit_DMA(&huart1, (uint8_t *)data, len);
    return true;
}

/* ======================== 中断处理 ======================== */

/**
 * @brief  USART1 中断: 处理 IDLE 空闲中断
 */
void BSP_UART1_IRQHandler(void)
{
    if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_IDLE))
    {
        /* 清除 IDLE 标志: 先读 SR 再读 DR */
        __HAL_UART_CLEAR_IDLEFLAG(&huart1);

        /* 停止 DMA 接收以获取已接收长度 */
        HAL_UART_DMAStop(&huart1);

        /* 计算本帧长度 */
        uint16_t rx_len = BSP_UART1_RX_BUF_SIZE - __HAL_DMA_GET_COUNTER(huart1.hdmarx);

        if (rx_len > 0 && rx_callback != NULL)
        {
            rx_callback(uart1_rx_buf, rx_len);
        }

        /* 重新启动 DMA 接收 */
        memset(uart1_rx_buf, 0, BSP_UART1_RX_BUF_SIZE);
        HAL_UART_Receive_DMA(&huart1, uart1_rx_buf, BSP_UART1_RX_BUF_SIZE);
    }

    /* 处理其他 UART 中断 (溢出等) */
    HAL_UART_IRQHandler(&huart1);
}

/**
 * @brief  DMA2 Stream7 中断 (USART1 TX 完成)
 */
void BSP_UART1_DMA_TX_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&hdma_usart1_tx);
    uart1_tx_busy = 0;
}

/**
 * @brief  DMA2 Stream5 中断 (USART1 RX 完成/半完成)
 */
void BSP_UART1_DMA_RX_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&hdma_usart1_rx);
}

