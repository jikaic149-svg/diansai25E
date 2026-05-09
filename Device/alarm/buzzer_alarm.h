#ifndef BUZZER_ALARM_H
#define BUZZER_ALARM_H

/**
 * @file buzzer_alarm.h
 * @brief 蜂鸣器报警模块（PWM抽象，支持优先级报警）
 * @origin 云台 modules/alarm/buzzer.h
 */

#include <stdint.h>

#define BUZZER_DEVICE_CNT 5

#define DoFreq 523
#define ReFreq 587
#define MiFreq 659
#define FaFreq 698
#define SoFreq 784
#define LaFreq 880
#define SiFreq 988

typedef enum
{
    OCTAVE_1 = 0, OCTAVE_2, OCTAVE_3, OCTAVE_4,
    OCTAVE_5, OCTAVE_6, OCTAVE_7, OCTAVE_8,
} octave_e;

typedef enum
{
    ALARM_LEVEL_HIGH = 0,
    ALARM_LEVEL_ABOVE_MEDIUM,
    ALARM_LEVEL_MEDIUM,
    ALARM_LEVEL_BELOW_MEDIUM,
    ALARM_LEVEL_LOW,
} AlarmLevel_e;

typedef enum { ALARM_OFF = 0, ALARM_ON } AlarmState_e;

typedef struct
{
    AlarmLevel_e alarm_level;
    octave_e octave;
    float loudness;
} Buzzer_config_s;

typedef struct
{
    float loudness;
    octave_e octave;
    AlarmLevel_e alarm_level;
    AlarmState_e alarm_state;
} BuzzerAlarmInstance;

/**
 * @brief PWM占空比设置函数指针
 * @param duty 占空比(0~1)
 */
typedef void (*BuzzerSetDuty_t)(float duty);

/**
 * @brief PWM周期设置函数指针
 * @param period 周期(秒)
 */
typedef void (*BuzzerSetPeriod_t)(float period);

/**
 * @brief 初始化蜂鸣器，注册PWM控制函数
 */
void BuzzerAlarmInit(BuzzerSetDuty_t set_duty, BuzzerSetPeriod_t set_period);

/**
 * @brief 注册一个报警器
 */
BuzzerAlarmInstance *BuzzerAlarmRegister(Buzzer_config_s *config);

/**
 * @brief 设置报警状态
 */
void BuzzerAlarmSetStatus(BuzzerAlarmInstance *buzzer, AlarmState_e state);

/**
 * @brief 蜂鸣器任务,需要被周期调用
 */
void BuzzerAlarmTask(void);

#endif
