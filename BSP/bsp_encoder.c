/**
 * @file  bsp_encoder.c
 * @brief BSP 编码器驱动实现
 *        软件 4 倍频正交解码：每个 A/B 相的上升沿和下降沿都计数
 *        通过 GPIOB 共享中断处理 4 个编码器引脚
 *
 *        引脚分配 (SysConfig):
 *          左编码器: A=PB12, B=PB6
 *          右编码器: A=PB8,  B=PB9
 */
#include "bsp_encoder.h"
#include "ti_msp_dl_config.h"

/* ===== 内部计数器（volatile，中断中修改） ===== */
static volatile int32_t s_left_count  = 0;
static volatile int32_t s_right_count = 0;

/* 上次读取时的快照（用于计算差值） */
static int32_t s_left_last  = 0;
static int32_t s_right_last = 0;

void BSP_Encoder_Init(void)
{
    /* GPIO 引脚和中断使能已由 SysConfig (SYSCFG_DL_GPIO_init) 完成 */
    /* 这里只需使能 GPIOB 的 NVIC 中断 */
    NVIC_EnableIRQ(GPIO_MULTIPLE_GPIOB_INT_IRQN);
}

/**
 * @brief GPIOB 中断服务 — 软件 4 倍频正交解码
 *
 * 正交编码原理 (4x 解码):
 *   A 上升沿: B=0 → 正转(+1), B=1 → 反转(-1)
 *   A 下降沿: B=1 → 正转(+1), B=0 → 反转(-1)
 *   B 上升沿: A=1 → 正转(+1), A=0 → 反转(-1)
 *   B 下降沿: A=0 → 正转(+1), A=1 → 反转(-1)
 */
void BSP_Encoder_GPIOB_IRQHandler(void)
{
    /* 读取中断索引（自动清除对应标志） */
    DL_GPIO_IIDX iidx = DL_GPIO_getPendingInterrupt(GPIOB);

    switch (iidx)
    {
        /* ===== 左编码器 ===== */
        case ENCODER_LEFT_A_ENC_L_A_IIDX:  /* A 相边沿 (PB12) */
        {
            uint32_t a = DL_GPIO_readPins(ENCODER_LEFT_A_PORT, ENCODER_LEFT_A_ENC_L_A_PIN);
            uint32_t b = DL_GPIO_readPins(ENCODER_LEFT_B_PORT, ENCODER_LEFT_B_ENC_L_B_PIN);
            if (a) {  /* A 上升沿 */
                s_left_count += (b ? -1 : 1);
            } else {  /* A 下降沿 */
                s_left_count += (b ? 1 : -1);
            }
            break;
        }
        case ENCODER_LEFT_B_ENC_L_B_IIDX:  /* B 相边沿 (PB6) */
        {
            uint32_t a = DL_GPIO_readPins(ENCODER_LEFT_A_PORT, ENCODER_LEFT_A_ENC_L_A_PIN);
            uint32_t b = DL_GPIO_readPins(ENCODER_LEFT_B_PORT, ENCODER_LEFT_B_ENC_L_B_PIN);
            if (b) {  /* B 上升沿 */
                s_left_count += (a ? 1 : -1);
            } else {  /* B 下降沿 */
                s_left_count += (a ? -1 : 1);
            }
            break;
        }

        /* ===== 右编码器 ===== */
        case ENCODER_RIGHT_A_ENC_R_A_IIDX:  /* A 相边沿 (PB8) */
        {
            uint32_t a = DL_GPIO_readPins(ENCODER_RIGHT_A_PORT, ENCODER_RIGHT_A_ENC_R_A_PIN);
            uint32_t b = DL_GPIO_readPins(ENCODER_RIGHT_B_PORT, ENCODER_RIGHT_B_ENC_R_B_PIN);
            if (a) {
                s_right_count += (b ? -1 : 1);
            } else {
                s_right_count += (b ? 1 : -1);
            }
            break;
        }
        case ENCODER_RIGHT_B_ENC_R_B_IIDX:  /* B 相边沿 (PB9) */
        {
            uint32_t a = DL_GPIO_readPins(ENCODER_RIGHT_A_PORT, ENCODER_RIGHT_A_ENC_R_A_PIN);
            uint32_t b = DL_GPIO_readPins(ENCODER_RIGHT_B_PORT, ENCODER_RIGHT_B_ENC_R_B_PIN);
            if (b) {
                s_right_count += (a ? 1 : -1);
            } else {
                s_right_count += (a ? -1 : 1);
            }
            break;
        }

        default:
            break;
    }
}

int16_t BSP_Encoder_GetLeftDiff(void)
{
    int32_t now = s_left_count;
    int16_t diff = (int16_t)(now - s_left_last);
    s_left_last = now;
    return diff;
}

int16_t BSP_Encoder_GetRightDiff(void)
{
    int32_t now = s_right_count;
    int16_t diff = (int16_t)(now - s_right_last);
    s_right_last = now;
    return diff;
}
