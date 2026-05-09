/**
 * @file bsp_pwm.c
 * @brief BSP PWM 实现：TIMG7 双通道 PWM 输出（纯整数运算，M0+ 零浮点开销）
 */
#include "bsp_pwm.h"
#include "ti_msp_dl_config.h"

void BSP_PWM_Init(void)
{
    BSP_PWM_SetDutyBoth(0, 0, 1000);
    DL_TimerG_startCounter(PWM_MOTOR_INST);
}

void BSP_PWM_SetDuty(uint8_t channel, uint16_t duty, uint16_t max_duty)
{
    if (max_duty == 0) return;
    if (duty > max_duty) duty = max_duty;

    uint32_t load = DL_TimerG_getLoadValue(PWM_MOTOR_INST);

    /*
     * 纠正：MSPM0 Edge-Align Down-Count PWM (INIT_VAL_LOW 常规动作)：
     *   在 ZERO (即周期起始/重载) 时拉 HIGH，在 counter == CC 时拉 LOW。
     *   此时：
     *   CC = LOAD → 下一瞬间即拉 LOW → 0% 占空比 
     *   CC = 0    → 一直等到最后才拉 LOW → 100% 占空比
     *
     *   因此：真实比较值 = 装载值 - (装载值 * 目标占空比 / 最高占空比)
     */
    uint32_t compare = load - ((uint32_t)load * duty / max_duty);

    if (channel == PWM_CHANNEL_0) {
        DL_TimerG_setCaptureCompareValue(PWM_MOTOR_INST, compare,
            DL_TIMER_CC_0_INDEX);
    } else if (channel == PWM_CHANNEL_1) {
        DL_TimerG_setCaptureCompareValue(PWM_MOTOR_INST, compare,
            DL_TIMER_CC_1_INDEX);
    }
}

void BSP_PWM_SetDutyBoth(uint16_t duty0, uint16_t duty1, uint16_t max_duty)
{
    BSP_PWM_SetDuty(PWM_CHANNEL_0, duty0, max_duty);
    BSP_PWM_SetDuty(PWM_CHANNEL_1, duty1, max_duty);
}
