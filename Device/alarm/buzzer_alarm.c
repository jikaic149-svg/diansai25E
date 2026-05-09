/**
 * @file buzzer_alarm.c
 * @brief 蜂鸣器报警模块实现（PWM抽象）
 */

#include "buzzer_alarm.h"
#include <stdlib.h>
#include <string.h>

static BuzzerSetDuty_t   buzzer_set_duty   = (void *)0;
static BuzzerSetPeriod_t buzzer_set_period = (void *)0;
static BuzzerAlarmInstance *buzzer_list[BUZZER_DEVICE_CNT] = {0};

void BuzzerAlarmInit(BuzzerSetDuty_t set_duty, BuzzerSetPeriod_t set_period)
{
    buzzer_set_duty   = set_duty;
    buzzer_set_period = set_period;
}

BuzzerAlarmInstance *BuzzerAlarmRegister(Buzzer_config_s *config)
{
    if (config->alarm_level >= BUZZER_DEVICE_CNT)
        return (void *)0;

    BuzzerAlarmInstance *inst = (BuzzerAlarmInstance *)malloc(sizeof(BuzzerAlarmInstance));
    memset(inst, 0, sizeof(BuzzerAlarmInstance));

    inst->alarm_level = config->alarm_level;
    inst->loudness    = config->loudness;
    inst->octave      = config->octave;
    inst->alarm_state = ALARM_OFF;

    buzzer_list[config->alarm_level] = inst;
    return inst;
}

void BuzzerAlarmSetStatus(BuzzerAlarmInstance *buzzer, AlarmState_e state)
{
    buzzer->alarm_state = state;
}

void BuzzerAlarmTask(void)
{
    if (!buzzer_set_duty || !buzzer_set_period)
        return;

    BuzzerAlarmInstance *buzz;
    for (int i = 0; i < BUZZER_DEVICE_CNT; ++i)
    {
        buzz = buzzer_list[i];
        if (!buzz || buzz->alarm_level > ALARM_LEVEL_LOW)
            continue;

        if (buzz->alarm_state == ALARM_OFF)
        {
            buzzer_set_duty(0);
        }
        else
        {
            buzzer_set_duty(buzz->loudness);
            switch (buzz->octave)
            {
            case OCTAVE_1: buzzer_set_period((float)1 / DoFreq); break;
            case OCTAVE_2: buzzer_set_period((float)1 / ReFreq); break;
            case OCTAVE_3: buzzer_set_period((float)1 / MiFreq); break;
            case OCTAVE_4: buzzer_set_period((float)1 / FaFreq); break;
            case OCTAVE_5: buzzer_set_period((float)1 / SoFreq); break;
            case OCTAVE_6: buzzer_set_period((float)1 / LaFreq); break;
            case OCTAVE_7: buzzer_set_period((float)1 / SiFreq); break;
            default: break;
            }
            break; // 只播放最高优先级的报警
        }
    }
}
