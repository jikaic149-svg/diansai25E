/*
 * Copyright (c) 2023, Texas Instruments Incorporated - http://www.ti.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ============ ti_msp_dl_config.h =============
 *  Configured MSPM0 DriverLib module declarations
 *
 *  DO NOT EDIT - This file is generated for the MSPM0G350X
 *  by the SysConfig tool.
 */
#ifndef ti_msp_dl_config_h
#define ti_msp_dl_config_h

#define CONFIG_MSPM0G350X
#define CONFIG_MSPM0G3507

#if defined(__ti_version__) || defined(__TI_COMPILER_VERSION__)
#define SYSCONFIG_WEAK __attribute__((weak))
#elif defined(__IAR_SYSTEMS_ICC__)
#define SYSCONFIG_WEAK __weak
#elif defined(__GNUC__)
#define SYSCONFIG_WEAK __attribute__((weak))
#endif

#include <ti/devices/msp/msp.h>
#include <ti/driverlib/driverlib.h>
#include <ti/driverlib/m0p/dl_core.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  ======== SYSCFG_DL_init ========
 *  Perform all required MSP DL initialization
 *
 *  This function should be called once at a point before any use of
 *  MSP DL.
 */


/* clang-format off */

#define POWER_STARTUP_DELAY                                                (16)


#define CPUCLK_FREQ                                                     32000000



/* Defines for PWM_MOTOR */
#define PWM_MOTOR_INST                                                     TIMG7
#define PWM_MOTOR_INST_IRQHandler                               TIMG7_IRQHandler
#define PWM_MOTOR_INST_INT_IRQN                                 (TIMG7_INT_IRQn)
#define PWM_MOTOR_INST_CLK_FREQ                                         32000000
/* GPIO defines for channel 0 */
#define GPIO_PWM_MOTOR_C0_PORT                                             GPIOA
#define GPIO_PWM_MOTOR_C0_PIN                                     DL_GPIO_PIN_17
#define GPIO_PWM_MOTOR_C0_IOMUX                                  (IOMUX_PINCM39)
#define GPIO_PWM_MOTOR_C0_IOMUX_FUNC                 IOMUX_PINCM39_PF_TIMG7_CCP0
#define GPIO_PWM_MOTOR_C0_IDX                                DL_TIMER_CC_0_INDEX
/* GPIO defines for channel 1 */
#define GPIO_PWM_MOTOR_C1_PORT                                             GPIOA
#define GPIO_PWM_MOTOR_C1_PIN                                     DL_GPIO_PIN_18
#define GPIO_PWM_MOTOR_C1_IOMUX                                  (IOMUX_PINCM40)
#define GPIO_PWM_MOTOR_C1_IOMUX_FUNC                 IOMUX_PINCM40_PF_TIMG7_CCP1
#define GPIO_PWM_MOTOR_C1_IDX                                DL_TIMER_CC_1_INDEX



/* Defines for SCHEDULE_TIMER */
#define SCHEDULE_TIMER_INST                                              (TIMG0)
#define SCHEDULE_TIMER_INST_IRQHandler                          TIMG0_IRQHandler
#define SCHEDULE_TIMER_INST_INT_IRQN                            (TIMG0_INT_IRQn)
#define SCHEDULE_TIMER_INST_LOAD_VALUE                                  (31999U)
/* Defines for QEI_SAMPLE_TIMER */
#define QEI_SAMPLE_TIMER_INST                                           (TIMG12)
#define QEI_SAMPLE_TIMER_INST_IRQHandler                       TIMG12_IRQHandler
#define QEI_SAMPLE_TIMER_INST_INT_IRQN                         (TIMG12_INT_IRQn)
#define QEI_SAMPLE_TIMER_INST_LOAD_VALUE                                (31999U)



/* Defines for UART_WIT */
#define UART_WIT_INST                                                      UART1
#define UART_WIT_INST_FREQUENCY                                         32000000
#define UART_WIT_INST_IRQHandler                                UART1_IRQHandler
#define UART_WIT_INST_INT_IRQN                                    UART1_INT_IRQn
#define GPIO_UART_WIT_RX_PORT                                              GPIOB
#define GPIO_UART_WIT_RX_PIN                                       DL_GPIO_PIN_7
#define GPIO_UART_WIT_IOMUX_RX                                   (IOMUX_PINCM24)
#define GPIO_UART_WIT_IOMUX_RX_FUNC                    IOMUX_PINCM24_PF_UART1_RX
#define UART_WIT_BAUD_RATE                                              (115200)
#define UART_WIT_IBRD_32_MHZ_115200_BAUD                                    (17)
#define UART_WIT_FBRD_32_MHZ_115200_BAUD                                    (23)
/* Defines for UART_Blue */
#define UART_Blue_INST                                                     UART0
#define UART_Blue_INST_FREQUENCY                                        32000000
#define UART_Blue_INST_IRQHandler                               UART0_IRQHandler
#define UART_Blue_INST_INT_IRQN                                   UART0_INT_IRQn
#define GPIO_UART_Blue_RX_PORT                                             GPIOB
#define GPIO_UART_Blue_TX_PORT                                             GPIOB
#define GPIO_UART_Blue_RX_PIN                                      DL_GPIO_PIN_1
#define GPIO_UART_Blue_TX_PIN                                      DL_GPIO_PIN_0
#define GPIO_UART_Blue_IOMUX_RX                                  (IOMUX_PINCM13)
#define GPIO_UART_Blue_IOMUX_TX                                  (IOMUX_PINCM12)
#define GPIO_UART_Blue_IOMUX_RX_FUNC                   IOMUX_PINCM13_PF_UART0_RX
#define GPIO_UART_Blue_IOMUX_TX_FUNC                   IOMUX_PINCM12_PF_UART0_TX
#define UART_Blue_BAUD_RATE                                             (115200)
#define UART_Blue_IBRD_32_MHZ_115200_BAUD                                   (17)
#define UART_Blue_FBRD_32_MHZ_115200_BAUD                                   (23)
/* Defines for UART_Track */
#define UART_Track_INST                                                    UART2
#define UART_Track_INST_FREQUENCY                                       32000000
#define UART_Track_INST_IRQHandler                              UART2_IRQHandler
#define UART_Track_INST_INT_IRQN                                  UART2_INT_IRQn
#define GPIO_UART_Track_RX_PORT                                            GPIOB
#define GPIO_UART_Track_TX_PORT                                            GPIOB
#define GPIO_UART_Track_RX_PIN                                    DL_GPIO_PIN_16
#define GPIO_UART_Track_TX_PIN                                    DL_GPIO_PIN_15
#define GPIO_UART_Track_IOMUX_RX                                 (IOMUX_PINCM33)
#define GPIO_UART_Track_IOMUX_TX                                 (IOMUX_PINCM32)
#define GPIO_UART_Track_IOMUX_RX_FUNC                  IOMUX_PINCM33_PF_UART2_RX
#define GPIO_UART_Track_IOMUX_TX_FUNC                  IOMUX_PINCM32_PF_UART2_TX
#define UART_Track_BAUD_RATE                                            (115200)
#define UART_Track_IBRD_32_MHZ_115200_BAUD                                  (17)
#define UART_Track_FBRD_32_MHZ_115200_BAUD                                  (23)





/* Defines for DMA_WIT */
#define DMA_WIT_CHAN_ID                                                      (4)
#define UART_WIT_INST_DMA_TRIGGER                            (DMA_UART1_RX_TRIG)
/* Defines for DMA_RX_Blue */
#define DMA_RX_Blue_CHAN_ID                                                  (3)
#define UART_Blue_INST_DMA_TRIGGER_0                         (DMA_UART0_RX_TRIG)
/* Defines for DMA_TX_Blue */
#define DMA_TX_Blue_CHAN_ID                                                  (2)
#define UART_Blue_INST_DMA_TRIGGER_1                         (DMA_UART0_TX_TRIG)
/* Defines for DMA_RX_Track */
#define DMA_RX_Track_CHAN_ID                                                 (1)
#define UART_Track_INST_DMA_TRIGGER_0                        (DMA_UART2_RX_TRIG)
/* Defines for DMA_TX_Track */
#define DMA_TX_Track_CHAN_ID                                                 (0)
#define UART_Track_INST_DMA_TRIGGER_1                        (DMA_UART2_TX_TRIG)


/* Port definition for Pin Group ENCODER_RIGHT_A */
#define ENCODER_RIGHT_A_PORT                                             (GPIOB)

/* Defines for ENC_R_A: GPIOB.8 with pinCMx 25 on package pin 60 */
// groups represented: ["ENCODER_RIGHT_B","ENCODER_LEFT_A","ENCODER_LEFT_B","ENCODER_RIGHT_A"]
// pins affected: ["ENC_R_B","ENC_L_A","ENC_L_B","ENC_R_A"]
#define GPIO_MULTIPLE_GPIOB_INT_IRQN                            (GPIOB_INT_IRQn)
#define GPIO_MULTIPLE_GPIOB_INT_IIDX            (DL_INTERRUPT_GROUP1_IIDX_GPIOB)
#define ENCODER_RIGHT_A_ENC_R_A_IIDX                         (DL_GPIO_IIDX_DIO8)
#define ENCODER_RIGHT_A_ENC_R_A_PIN                              (DL_GPIO_PIN_8)
#define ENCODER_RIGHT_A_ENC_R_A_IOMUX                            (IOMUX_PINCM25)
/* Port definition for Pin Group ENCODER_RIGHT_B */
#define ENCODER_RIGHT_B_PORT                                             (GPIOB)

/* Defines for ENC_R_B: GPIOB.9 with pinCMx 26 on package pin 61 */
#define ENCODER_RIGHT_B_ENC_R_B_IIDX                         (DL_GPIO_IIDX_DIO9)
#define ENCODER_RIGHT_B_ENC_R_B_PIN                              (DL_GPIO_PIN_9)
#define ENCODER_RIGHT_B_ENC_R_B_IOMUX                            (IOMUX_PINCM26)
/* Port definition for Pin Group ENCODER_LEFT_A */
#define ENCODER_LEFT_A_PORT                                              (GPIOB)

/* Defines for ENC_L_A: GPIOB.12 with pinCMx 29 on package pin 64 */
#define ENCODER_LEFT_A_ENC_L_A_IIDX                         (DL_GPIO_IIDX_DIO12)
#define ENCODER_LEFT_A_ENC_L_A_PIN                              (DL_GPIO_PIN_12)
#define ENCODER_LEFT_A_ENC_L_A_IOMUX                             (IOMUX_PINCM29)
/* Port definition for Pin Group ENCODER_LEFT_B */
#define ENCODER_LEFT_B_PORT                                              (GPIOB)

/* Defines for ENC_L_B: GPIOB.6 with pinCMx 23 on package pin 58 */
#define ENCODER_LEFT_B_ENC_L_B_IIDX                          (DL_GPIO_IIDX_DIO6)
#define ENCODER_LEFT_B_ENC_L_B_PIN                               (DL_GPIO_PIN_6)
#define ENCODER_LEFT_B_ENC_L_B_IOMUX                             (IOMUX_PINCM23)
/* Defines for STBY: GPIOA.12 with pinCMx 34 on package pin 5 */
#define GPIO_MOTOR_DIR_STBY_PORT                                         (GPIOA)
#define GPIO_MOTOR_DIR_STBY_PIN                                 (DL_GPIO_PIN_12)
#define GPIO_MOTOR_DIR_STBY_IOMUX                                (IOMUX_PINCM34)
/* Defines for AIN1: GPIOA.13 with pinCMx 35 on package pin 6 */
#define GPIO_MOTOR_DIR_AIN1_PORT                                         (GPIOA)
#define GPIO_MOTOR_DIR_AIN1_PIN                                 (DL_GPIO_PIN_13)
#define GPIO_MOTOR_DIR_AIN1_IOMUX                                (IOMUX_PINCM35)
/* Defines for AIN2: GPIOB.4 with pinCMx 17 on package pin 52 */
#define GPIO_MOTOR_DIR_AIN2_PORT                                         (GPIOB)
#define GPIO_MOTOR_DIR_AIN2_PIN                                  (DL_GPIO_PIN_4)
#define GPIO_MOTOR_DIR_AIN2_IOMUX                                (IOMUX_PINCM17)
/* Defines for BIN1: GPIOB.2 with pinCMx 15 on package pin 50 */
#define GPIO_MOTOR_DIR_BIN1_PORT                                         (GPIOB)
#define GPIO_MOTOR_DIR_BIN1_PIN                                  (DL_GPIO_PIN_2)
#define GPIO_MOTOR_DIR_BIN1_IOMUX                                (IOMUX_PINCM15)
/* Defines for BIN2: GPIOB.3 with pinCMx 16 on package pin 51 */
#define GPIO_MOTOR_DIR_BIN2_PORT                                         (GPIOB)
#define GPIO_MOTOR_DIR_BIN2_PIN                                  (DL_GPIO_PIN_3)
#define GPIO_MOTOR_DIR_BIN2_IOMUX                                (IOMUX_PINCM16)
/* Defines for KEY_S2: GPIOB.21 with pinCMx 49 on package pin 20 */
#define GPIO_UI_KEY_S2_PORT                                              (GPIOB)
#define GPIO_UI_KEY_S2_PIN                                      (DL_GPIO_PIN_21)
#define GPIO_UI_KEY_S2_IOMUX                                     (IOMUX_PINCM49)
/* Defines for LED: GPIOA.0 with pinCMx 1 on package pin 33 */
#define GPIO_UI_LED_PORT                                                 (GPIOA)
#define GPIO_UI_LED_PIN                                          (DL_GPIO_PIN_0)
#define GPIO_UI_LED_IOMUX                                         (IOMUX_PINCM1)


/* clang-format on */

void SYSCFG_DL_init(void);
void SYSCFG_DL_initPower(void);
void SYSCFG_DL_GPIO_init(void);
void SYSCFG_DL_SYSCTL_init(void);
void SYSCFG_DL_PWM_MOTOR_init(void);
void SYSCFG_DL_SCHEDULE_TIMER_init(void);
void SYSCFG_DL_QEI_SAMPLE_TIMER_init(void);
void SYSCFG_DL_UART_WIT_init(void);
void SYSCFG_DL_UART_Blue_init(void);
void SYSCFG_DL_UART_Track_init(void);
void SYSCFG_DL_DMA_init(void);


bool SYSCFG_DL_saveConfiguration(void);
bool SYSCFG_DL_restoreConfiguration(void);

#ifdef __cplusplus
}
#endif

#endif /* ti_msp_dl_config_h */
