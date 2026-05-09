/*
 * Copyright (c) 2023, Texas Instruments Incorporated
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
 *  ============ ti_msp_dl_config.c =============
 *  Configured MSPM0 DriverLib module definitions
 *
 *  DO NOT EDIT - This file is generated for the MSPM0G350X
 *  by the SysConfig tool.
 */

#include "ti_msp_dl_config.h"

DL_TimerG_backupConfig gPWM_MOTORBackup;

/*
 *  ======== SYSCFG_DL_init ========
 *  Perform any initialization needed before using any board APIs
 */
SYSCONFIG_WEAK void SYSCFG_DL_init(void)
{
    SYSCFG_DL_initPower();
    SYSCFG_DL_GPIO_init();
    /* Module-Specific Initializations*/
    SYSCFG_DL_SYSCTL_init();
    SYSCFG_DL_PWM_MOTOR_init();
    SYSCFG_DL_SCHEDULE_TIMER_init();
    SYSCFG_DL_QEI_SAMPLE_TIMER_init();
    SYSCFG_DL_UART_WIT_init();
    SYSCFG_DL_UART_Blue_init();
    SYSCFG_DL_UART_Track_init();
    SYSCFG_DL_DMA_init();
    /* Ensure backup structures have no valid state */
	gPWM_MOTORBackup.backupRdy 	= false;



}
/*
 * User should take care to save and restore register configuration in application.
 * See Retention Configuration section for more details.
 */
SYSCONFIG_WEAK bool SYSCFG_DL_saveConfiguration(void)
{
    bool retStatus = true;

	retStatus &= DL_TimerG_saveConfiguration(PWM_MOTOR_INST, &gPWM_MOTORBackup);

    return retStatus;
}


SYSCONFIG_WEAK bool SYSCFG_DL_restoreConfiguration(void)
{
    bool retStatus = true;

	retStatus &= DL_TimerG_restoreConfiguration(PWM_MOTOR_INST, &gPWM_MOTORBackup, false);

    return retStatus;
}

SYSCONFIG_WEAK void SYSCFG_DL_initPower(void)
{
    DL_GPIO_reset(GPIOA);
    DL_GPIO_reset(GPIOB);
    DL_TimerG_reset(PWM_MOTOR_INST);
    DL_TimerG_reset(SCHEDULE_TIMER_INST);
    DL_TimerG_reset(QEI_SAMPLE_TIMER_INST);
    DL_UART_Main_reset(UART_WIT_INST);
    DL_UART_Main_reset(UART_Blue_INST);
    DL_UART_Main_reset(UART_Track_INST);


    DL_GPIO_enablePower(GPIOA);
    DL_GPIO_enablePower(GPIOB);
    DL_TimerG_enablePower(PWM_MOTOR_INST);
    DL_TimerG_enablePower(SCHEDULE_TIMER_INST);
    DL_TimerG_enablePower(QEI_SAMPLE_TIMER_INST);
    DL_UART_Main_enablePower(UART_WIT_INST);
    DL_UART_Main_enablePower(UART_Blue_INST);
    DL_UART_Main_enablePower(UART_Track_INST);

    delay_cycles(POWER_STARTUP_DELAY);
}

SYSCONFIG_WEAK void SYSCFG_DL_GPIO_init(void)
{

    DL_GPIO_initPeripheralOutputFunction(GPIO_PWM_MOTOR_C0_IOMUX,GPIO_PWM_MOTOR_C0_IOMUX_FUNC);
    DL_GPIO_enableOutput(GPIO_PWM_MOTOR_C0_PORT, GPIO_PWM_MOTOR_C0_PIN);
    DL_GPIO_initPeripheralOutputFunction(GPIO_PWM_MOTOR_C1_IOMUX,GPIO_PWM_MOTOR_C1_IOMUX_FUNC);
    DL_GPIO_enableOutput(GPIO_PWM_MOTOR_C1_PORT, GPIO_PWM_MOTOR_C1_PIN);

    DL_GPIO_initPeripheralInputFunction(
        GPIO_UART_WIT_IOMUX_RX, GPIO_UART_WIT_IOMUX_RX_FUNC);
    DL_GPIO_initPeripheralOutputFunction(
        GPIO_UART_Blue_IOMUX_TX, GPIO_UART_Blue_IOMUX_TX_FUNC);
    DL_GPIO_initPeripheralInputFunction(
        GPIO_UART_Blue_IOMUX_RX, GPIO_UART_Blue_IOMUX_RX_FUNC);
    DL_GPIO_initPeripheralOutputFunction(
        GPIO_UART_Track_IOMUX_TX, GPIO_UART_Track_IOMUX_TX_FUNC);
    DL_GPIO_initPeripheralInputFunction(
        GPIO_UART_Track_IOMUX_RX, GPIO_UART_Track_IOMUX_RX_FUNC);

    DL_GPIO_initDigitalInputFeatures(ENCODER_RIGHT_A_ENC_R_A_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(ENCODER_RIGHT_B_ENC_R_B_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(ENCODER_LEFT_A_ENC_L_A_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(ENCODER_LEFT_B_ENC_L_B_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalOutput(GPIO_MOTOR_DIR_STBY_IOMUX);

    DL_GPIO_initDigitalOutput(GPIO_MOTOR_DIR_AIN1_IOMUX);

    DL_GPIO_initDigitalOutput(GPIO_MOTOR_DIR_AIN2_IOMUX);

    DL_GPIO_initDigitalOutput(GPIO_MOTOR_DIR_BIN1_IOMUX);

    DL_GPIO_initDigitalOutput(GPIO_MOTOR_DIR_BIN2_IOMUX);

    DL_GPIO_initDigitalInputFeatures(GPIO_UI_KEY_S2_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalOutput(GPIO_UI_LED_IOMUX);

    DL_GPIO_clearPins(GPIOA, GPIO_MOTOR_DIR_STBY_PIN |
		GPIO_MOTOR_DIR_AIN1_PIN |
		GPIO_UI_LED_PIN);
    DL_GPIO_enableOutput(GPIOA, GPIO_MOTOR_DIR_STBY_PIN |
		GPIO_MOTOR_DIR_AIN1_PIN |
		GPIO_UI_LED_PIN);
    DL_GPIO_clearPins(GPIOB, GPIO_MOTOR_DIR_AIN2_PIN |
		GPIO_MOTOR_DIR_BIN1_PIN |
		GPIO_MOTOR_DIR_BIN2_PIN);
    DL_GPIO_enableOutput(GPIOB, GPIO_MOTOR_DIR_AIN2_PIN |
		GPIO_MOTOR_DIR_BIN1_PIN |
		GPIO_MOTOR_DIR_BIN2_PIN);
    DL_GPIO_setLowerPinsPolarity(GPIOB, DL_GPIO_PIN_8_EDGE_RISE_FALL |
		DL_GPIO_PIN_9_EDGE_RISE_FALL |
		DL_GPIO_PIN_12_EDGE_RISE_FALL |
		DL_GPIO_PIN_6_EDGE_RISE_FALL);
    DL_GPIO_clearInterruptStatus(GPIOB, ENCODER_RIGHT_A_ENC_R_A_PIN |
		ENCODER_RIGHT_B_ENC_R_B_PIN |
		ENCODER_LEFT_A_ENC_L_A_PIN |
		ENCODER_LEFT_B_ENC_L_B_PIN);
    DL_GPIO_enableInterrupt(GPIOB, ENCODER_RIGHT_A_ENC_R_A_PIN |
		ENCODER_RIGHT_B_ENC_R_B_PIN |
		ENCODER_LEFT_A_ENC_L_A_PIN |
		ENCODER_LEFT_B_ENC_L_B_PIN);

}


SYSCONFIG_WEAK void SYSCFG_DL_SYSCTL_init(void)
{

	//Low Power Mode is configured to be SLEEP0
    DL_SYSCTL_setBORThreshold(DL_SYSCTL_BOR_THRESHOLD_LEVEL_0);

    DL_SYSCTL_setSYSOSCFreq(DL_SYSCTL_SYSOSC_FREQ_BASE);
    /* Set default configuration */
    DL_SYSCTL_disableHFXT();
    DL_SYSCTL_disableSYSPLL();
    DL_SYSCTL_setULPCLKDivider(DL_SYSCTL_ULPCLK_DIV_1);
    DL_SYSCTL_setMCLKDivider(DL_SYSCTL_MCLK_DIVIDER_DISABLE);

}


/*
 * Timer clock configuration to be sourced by  / 1 (32000000 Hz)
 * timerClkFreq = (timerClkSrc / (timerClkDivRatio * (timerClkPrescale + 1)))
 *   32000000 Hz = 32000000 Hz / (1 * (0 + 1))
 */
static const DL_TimerG_ClockConfig gPWM_MOTORClockConfig = {
    .clockSel = DL_TIMER_CLOCK_BUSCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_1,
    .prescale = 0U
};

static const DL_TimerG_PWMConfig gPWM_MOTORConfig = {
    .pwmMode = DL_TIMER_PWM_MODE_EDGE_ALIGN,
    .period = 1600,
    .isTimerWithFourCC = false,
    .startTimer = DL_TIMER_STOP,
};

SYSCONFIG_WEAK void SYSCFG_DL_PWM_MOTOR_init(void) {

    DL_TimerG_setClockConfig(
        PWM_MOTOR_INST, (DL_TimerG_ClockConfig *) &gPWM_MOTORClockConfig);

    DL_TimerG_initPWMMode(
        PWM_MOTOR_INST, (DL_TimerG_PWMConfig *) &gPWM_MOTORConfig);

    // Set Counter control to the smallest CC index being used
    DL_TimerG_setCounterControl(PWM_MOTOR_INST,DL_TIMER_CZC_CCCTL0_ZCOND,DL_TIMER_CAC_CCCTL0_ACOND,DL_TIMER_CLC_CCCTL0_LCOND);

    DL_TimerG_setCaptureCompareOutCtl(PWM_MOTOR_INST, DL_TIMER_CC_OCTL_INIT_VAL_LOW,
		DL_TIMER_CC_OCTL_INV_OUT_DISABLED, DL_TIMER_CC_OCTL_SRC_FUNCVAL,
		DL_TIMERG_CAPTURE_COMPARE_0_INDEX);

    DL_TimerG_setCaptCompUpdateMethod(PWM_MOTOR_INST, DL_TIMER_CC_UPDATE_METHOD_IMMEDIATE, DL_TIMERG_CAPTURE_COMPARE_0_INDEX);
    DL_TimerG_setCaptureCompareValue(PWM_MOTOR_INST, 1600, DL_TIMER_CC_0_INDEX);

    DL_TimerG_setCaptureCompareOutCtl(PWM_MOTOR_INST, DL_TIMER_CC_OCTL_INIT_VAL_LOW,
		DL_TIMER_CC_OCTL_INV_OUT_DISABLED, DL_TIMER_CC_OCTL_SRC_FUNCVAL,
		DL_TIMERG_CAPTURE_COMPARE_1_INDEX);

    DL_TimerG_setCaptCompUpdateMethod(PWM_MOTOR_INST, DL_TIMER_CC_UPDATE_METHOD_IMMEDIATE, DL_TIMERG_CAPTURE_COMPARE_1_INDEX);
    DL_TimerG_setCaptureCompareValue(PWM_MOTOR_INST, 1600, DL_TIMER_CC_1_INDEX);

    DL_TimerG_enableClock(PWM_MOTOR_INST);


    
    DL_TimerG_setCCPDirection(PWM_MOTOR_INST , DL_TIMER_CC0_OUTPUT | DL_TIMER_CC1_OUTPUT );


}



/*
 * Timer clock configuration to be sourced by BUSCLK /  (32000000 Hz)
 * timerClkFreq = (timerClkSrc / (timerClkDivRatio * (timerClkPrescale + 1)))
 *   32000000 Hz = 32000000 Hz / (1 * (0 + 1))
 */
static const DL_TimerG_ClockConfig gSCHEDULE_TIMERClockConfig = {
    .clockSel    = DL_TIMER_CLOCK_BUSCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_1,
    .prescale    = 0U,
};

/*
 * Timer load value (where the counter starts from) is calculated as (timerPeriod * timerClockFreq) - 1
 * SCHEDULE_TIMER_INST_LOAD_VALUE = (1 ms * 32000000 Hz) - 1
 */
static const DL_TimerG_TimerConfig gSCHEDULE_TIMERTimerConfig = {
    .period     = SCHEDULE_TIMER_INST_LOAD_VALUE,
    .timerMode  = DL_TIMER_TIMER_MODE_PERIODIC,
    .startTimer = DL_TIMER_STOP,
};

SYSCONFIG_WEAK void SYSCFG_DL_SCHEDULE_TIMER_init(void) {

    DL_TimerG_setClockConfig(SCHEDULE_TIMER_INST,
        (DL_TimerG_ClockConfig *) &gSCHEDULE_TIMERClockConfig);

    DL_TimerG_initTimerMode(SCHEDULE_TIMER_INST,
        (DL_TimerG_TimerConfig *) &gSCHEDULE_TIMERTimerConfig);
    DL_TimerG_enableInterrupt(SCHEDULE_TIMER_INST , DL_TIMERG_INTERRUPT_ZERO_EVENT);
    DL_TimerG_enableClock(SCHEDULE_TIMER_INST);





}

/*
 * Timer clock configuration to be sourced by BUSCLK /  (32000000 Hz)
 * timerClkFreq = (timerClkSrc / (timerClkDivRatio * (timerClkPrescale + 1)))
 *   32000000 Hz = 32000000 Hz / (1 * (0 + 1))
 */
static const DL_TimerG_ClockConfig gQEI_SAMPLE_TIMERClockConfig = {
    .clockSel    = DL_TIMER_CLOCK_BUSCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_1,
    .prescale    = 0U,
};

/*
 * Timer load value (where the counter starts from) is calculated as (timerPeriod * timerClockFreq) - 1
 * QEI_SAMPLE_TIMER_INST_LOAD_VALUE = (1 ms * 32000000 Hz) - 1
 */
static const DL_TimerG_TimerConfig gQEI_SAMPLE_TIMERTimerConfig = {
    .period     = QEI_SAMPLE_TIMER_INST_LOAD_VALUE,
    .timerMode  = DL_TIMER_TIMER_MODE_PERIODIC,
    .startTimer = DL_TIMER_STOP,
};

SYSCONFIG_WEAK void SYSCFG_DL_QEI_SAMPLE_TIMER_init(void) {

    DL_TimerG_setClockConfig(QEI_SAMPLE_TIMER_INST,
        (DL_TimerG_ClockConfig *) &gQEI_SAMPLE_TIMERClockConfig);

    DL_TimerG_initTimerMode(QEI_SAMPLE_TIMER_INST,
        (DL_TimerG_TimerConfig *) &gQEI_SAMPLE_TIMERTimerConfig);
    DL_TimerG_enableInterrupt(QEI_SAMPLE_TIMER_INST , DL_TIMERG_INTERRUPT_ZERO_EVENT);
    DL_TimerG_enableClock(QEI_SAMPLE_TIMER_INST);





}


static const DL_UART_Main_ClockConfig gUART_WITClockConfig = {
    .clockSel    = DL_UART_MAIN_CLOCK_BUSCLK,
    .divideRatio = DL_UART_MAIN_CLOCK_DIVIDE_RATIO_1
};

static const DL_UART_Main_Config gUART_WITConfig = {
    .mode        = DL_UART_MAIN_MODE_NORMAL,
    .direction   = DL_UART_MAIN_DIRECTION_RX,
    .flowControl = DL_UART_MAIN_FLOW_CONTROL_NONE,
    .parity      = DL_UART_MAIN_PARITY_NONE,
    .wordLength  = DL_UART_MAIN_WORD_LENGTH_8_BITS,
    .stopBits    = DL_UART_MAIN_STOP_BITS_ONE
};

SYSCONFIG_WEAK void SYSCFG_DL_UART_WIT_init(void)
{
    DL_UART_Main_setClockConfig(UART_WIT_INST, (DL_UART_Main_ClockConfig *) &gUART_WITClockConfig);

    DL_UART_Main_init(UART_WIT_INST, (DL_UART_Main_Config *) &gUART_WITConfig);
    /*
     * Configure baud rate by setting oversampling and baud rate divisors.
     *  Target baud rate: 115200
     *  Actual baud rate: 115211.52
     */
    DL_UART_Main_setOversampling(UART_WIT_INST, DL_UART_OVERSAMPLING_RATE_16X);
    DL_UART_Main_setBaudRateDivisor(UART_WIT_INST, UART_WIT_IBRD_32_MHZ_115200_BAUD, UART_WIT_FBRD_32_MHZ_115200_BAUD);


    /* Configure Interrupts */
    DL_UART_Main_enableInterrupt(UART_WIT_INST,
                                 DL_UART_MAIN_INTERRUPT_RX_TIMEOUT_ERROR);

    /* Configure DMA Receive Event */
    DL_UART_Main_enableDMAReceiveEvent(UART_WIT_INST, DL_UART_DMA_INTERRUPT_RX);
    /* Configure FIFOs */
    DL_UART_Main_enableFIFOs(UART_WIT_INST);
    DL_UART_Main_setRXFIFOThreshold(UART_WIT_INST, DL_UART_RX_FIFO_LEVEL_ONE_ENTRY);

    DL_UART_Main_setRXInterruptTimeout(UART_WIT_INST, 1);

    DL_UART_Main_enable(UART_WIT_INST);
}
static const DL_UART_Main_ClockConfig gUART_BlueClockConfig = {
    .clockSel    = DL_UART_MAIN_CLOCK_BUSCLK,
    .divideRatio = DL_UART_MAIN_CLOCK_DIVIDE_RATIO_1
};

static const DL_UART_Main_Config gUART_BlueConfig = {
    .mode        = DL_UART_MAIN_MODE_NORMAL,
    .direction   = DL_UART_MAIN_DIRECTION_TX_RX,
    .flowControl = DL_UART_MAIN_FLOW_CONTROL_NONE,
    .parity      = DL_UART_MAIN_PARITY_NONE,
    .wordLength  = DL_UART_MAIN_WORD_LENGTH_8_BITS,
    .stopBits    = DL_UART_MAIN_STOP_BITS_ONE
};

SYSCONFIG_WEAK void SYSCFG_DL_UART_Blue_init(void)
{
    DL_UART_Main_setClockConfig(UART_Blue_INST, (DL_UART_Main_ClockConfig *) &gUART_BlueClockConfig);

    DL_UART_Main_init(UART_Blue_INST, (DL_UART_Main_Config *) &gUART_BlueConfig);
    /*
     * Configure baud rate by setting oversampling and baud rate divisors.
     *  Target baud rate: 115200
     *  Actual baud rate: 115211.52
     */
    DL_UART_Main_setOversampling(UART_Blue_INST, DL_UART_OVERSAMPLING_RATE_16X);
    DL_UART_Main_setBaudRateDivisor(UART_Blue_INST, UART_Blue_IBRD_32_MHZ_115200_BAUD, UART_Blue_FBRD_32_MHZ_115200_BAUD);


    /* Configure Interrupts */
    DL_UART_Main_enableInterrupt(UART_Blue_INST,
                                 DL_UART_MAIN_INTERRUPT_RX_TIMEOUT_ERROR);

    /* Configure DMA Receive Event */
    DL_UART_Main_enableDMAReceiveEvent(UART_Blue_INST, DL_UART_DMA_INTERRUPT_RX);
    /* Configure DMA Transmit Event */
    DL_UART_Main_enableDMATransmitEvent(UART_Blue_INST);
    /* Configure FIFOs */
    DL_UART_Main_enableFIFOs(UART_Blue_INST);
    DL_UART_Main_setRXFIFOThreshold(UART_Blue_INST, DL_UART_RX_FIFO_LEVEL_ONE_ENTRY);
    DL_UART_Main_setTXFIFOThreshold(UART_Blue_INST, DL_UART_TX_FIFO_LEVEL_ONE_ENTRY);

    DL_UART_Main_setRXInterruptTimeout(UART_Blue_INST, 1);

    DL_UART_Main_enable(UART_Blue_INST);
}
static const DL_UART_Main_ClockConfig gUART_TrackClockConfig = {
    .clockSel    = DL_UART_MAIN_CLOCK_BUSCLK,
    .divideRatio = DL_UART_MAIN_CLOCK_DIVIDE_RATIO_1
};

static const DL_UART_Main_Config gUART_TrackConfig = {
    .mode        = DL_UART_MAIN_MODE_NORMAL,
    .direction   = DL_UART_MAIN_DIRECTION_TX_RX,
    .flowControl = DL_UART_MAIN_FLOW_CONTROL_NONE,
    .parity      = DL_UART_MAIN_PARITY_NONE,
    .wordLength  = DL_UART_MAIN_WORD_LENGTH_8_BITS,
    .stopBits    = DL_UART_MAIN_STOP_BITS_ONE
};

SYSCONFIG_WEAK void SYSCFG_DL_UART_Track_init(void)
{
    DL_UART_Main_setClockConfig(UART_Track_INST, (DL_UART_Main_ClockConfig *) &gUART_TrackClockConfig);

    DL_UART_Main_init(UART_Track_INST, (DL_UART_Main_Config *) &gUART_TrackConfig);
    /*
     * Configure baud rate by setting oversampling and baud rate divisors.
     *  Target baud rate: 115200
     *  Actual baud rate: 115211.52
     */
    DL_UART_Main_setOversampling(UART_Track_INST, DL_UART_OVERSAMPLING_RATE_16X);
    DL_UART_Main_setBaudRateDivisor(UART_Track_INST, UART_Track_IBRD_32_MHZ_115200_BAUD, UART_Track_FBRD_32_MHZ_115200_BAUD);


    /* Configure Interrupts */
    DL_UART_Main_enableInterrupt(UART_Track_INST,
                                 DL_UART_MAIN_INTERRUPT_RX_TIMEOUT_ERROR);

    /* Configure DMA Receive Event */
    DL_UART_Main_enableDMAReceiveEvent(UART_Track_INST, DL_UART_DMA_INTERRUPT_RX);
    /* Configure DMA Transmit Event */
    DL_UART_Main_enableDMATransmitEvent(UART_Track_INST);
    /* Configure FIFOs */
    DL_UART_Main_enableFIFOs(UART_Track_INST);
    DL_UART_Main_setRXFIFOThreshold(UART_Track_INST, DL_UART_RX_FIFO_LEVEL_ONE_ENTRY);
    DL_UART_Main_setTXFIFOThreshold(UART_Track_INST, DL_UART_TX_FIFO_LEVEL_ONE_ENTRY);

    DL_UART_Main_setRXInterruptTimeout(UART_Track_INST, 1);

    DL_UART_Main_enable(UART_Track_INST);
}

static const DL_DMA_Config gDMA_WITConfig = {
    .transferMode   = DL_DMA_SINGLE_TRANSFER_MODE,
    .extendedMode   = DL_DMA_NORMAL_MODE,
    .destIncrement  = DL_DMA_ADDR_INCREMENT,
    .srcIncrement   = DL_DMA_ADDR_UNCHANGED,
    .destWidth      = DL_DMA_WIDTH_BYTE,
    .srcWidth       = DL_DMA_WIDTH_BYTE,
    .trigger        = UART_WIT_INST_DMA_TRIGGER,
    .triggerType    = DL_DMA_TRIGGER_TYPE_EXTERNAL,
};

SYSCONFIG_WEAK void SYSCFG_DL_DMA_WIT_init(void)
{
    DL_DMA_initChannel(DMA, DMA_WIT_CHAN_ID , (DL_DMA_Config *) &gDMA_WITConfig);
}
static const DL_DMA_Config gDMA_RX_BlueConfig = {
    .transferMode   = DL_DMA_SINGLE_TRANSFER_MODE,
    .extendedMode   = DL_DMA_NORMAL_MODE,
    .destIncrement  = DL_DMA_ADDR_UNCHANGED,
    .srcIncrement   = DL_DMA_ADDR_UNCHANGED,
    .destWidth      = DL_DMA_WIDTH_BYTE,
    .srcWidth       = DL_DMA_WIDTH_BYTE,
    .trigger        = UART_Blue_INST_DMA_TRIGGER_0,
    .triggerType    = DL_DMA_TRIGGER_TYPE_EXTERNAL,
};

SYSCONFIG_WEAK void SYSCFG_DL_DMA_RX_Blue_init(void)
{
    DL_DMA_initChannel(DMA, DMA_RX_Blue_CHAN_ID , (DL_DMA_Config *) &gDMA_RX_BlueConfig);
}
static const DL_DMA_Config gDMA_TX_BlueConfig = {
    .transferMode   = DL_DMA_SINGLE_TRANSFER_MODE,
    .extendedMode   = DL_DMA_NORMAL_MODE,
    .destIncrement  = DL_DMA_ADDR_UNCHANGED,
    .srcIncrement   = DL_DMA_ADDR_INCREMENT,
    .destWidth      = DL_DMA_WIDTH_BYTE,
    .srcWidth       = DL_DMA_WIDTH_BYTE,
    .trigger        = UART_Blue_INST_DMA_TRIGGER_1,
    .triggerType    = DL_DMA_TRIGGER_TYPE_EXTERNAL,
};

SYSCONFIG_WEAK void SYSCFG_DL_DMA_TX_Blue_init(void)
{
    DL_DMA_initChannel(DMA, DMA_TX_Blue_CHAN_ID , (DL_DMA_Config *) &gDMA_TX_BlueConfig);
}
static const DL_DMA_Config gDMA_RX_TrackConfig = {
    .transferMode   = DL_DMA_SINGLE_TRANSFER_MODE,
    .extendedMode   = DL_DMA_NORMAL_MODE,
    .destIncrement  = DL_DMA_ADDR_INCREMENT,
    .srcIncrement   = DL_DMA_ADDR_UNCHANGED,
    .destWidth      = DL_DMA_WIDTH_BYTE,
    .srcWidth       = DL_DMA_WIDTH_BYTE,
    .trigger        = UART_Track_INST_DMA_TRIGGER_0,
    .triggerType    = DL_DMA_TRIGGER_TYPE_EXTERNAL,
};

SYSCONFIG_WEAK void SYSCFG_DL_DMA_RX_Track_init(void)
{
    DL_DMA_initChannel(DMA, DMA_RX_Track_CHAN_ID , (DL_DMA_Config *) &gDMA_RX_TrackConfig);
}
static const DL_DMA_Config gDMA_TX_TrackConfig = {
    .transferMode   = DL_DMA_SINGLE_TRANSFER_MODE,
    .extendedMode   = DL_DMA_NORMAL_MODE,
    .destIncrement  = DL_DMA_ADDR_UNCHANGED,
    .srcIncrement   = DL_DMA_ADDR_INCREMENT,
    .destWidth      = DL_DMA_WIDTH_BYTE,
    .srcWidth       = DL_DMA_WIDTH_BYTE,
    .trigger        = UART_Track_INST_DMA_TRIGGER_1,
    .triggerType    = DL_DMA_TRIGGER_TYPE_EXTERNAL,
};

SYSCONFIG_WEAK void SYSCFG_DL_DMA_TX_Track_init(void)
{
    DL_DMA_initChannel(DMA, DMA_TX_Track_CHAN_ID , (DL_DMA_Config *) &gDMA_TX_TrackConfig);
}
SYSCONFIG_WEAK void SYSCFG_DL_DMA_init(void){
    SYSCFG_DL_DMA_WIT_init();
    SYSCFG_DL_DMA_RX_Blue_init();
    SYSCFG_DL_DMA_TX_Blue_init();
    SYSCFG_DL_DMA_RX_Track_init();
    SYSCFG_DL_DMA_TX_Track_init();
}


