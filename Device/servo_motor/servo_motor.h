#ifndef SERVO_MOTOR_H
#define SERVO_MOTOR_H

/**
 * @file servo_motor.h
 * @brief 舵机驱动（支持PWM舵机和总线舵机，硬件抽象）
 * @origin 云台 modules/motor/servo_motor/servo_motor.h
 */

#include <stdint.h>

#define SERVO_MOTOR_CNT     7
#define Servo_Frame_First   0x55
#define Servo_Frame_Second  0x55
#define SERVO_MOVE_CMD      0x03
#define SERVO_UNLOAD_CMD    0x14
#define SERVO_POS_READ_CMD  0x15

typedef enum
{
    Servo_None_Type = 0,
    Bus_Servo  = 1,
    PWM_Servo  = 2,
} ServoType_e;

/**
 * @brief PWM占空比设置函数指针
 */
typedef void (*Servo_PWMSetDuty_t)(float duty);

/**
 * @brief UART发送函数指针（总线舵机用）
 * @param data  发送数据
 * @param len   数据长度
 */
typedef void (*Servo_UartSend_t)(const uint8_t *data, uint16_t len);

typedef struct
{
    ServoType_e servo_type;
    uint8_t servo_id;
    /* PWM舵机 */
    Servo_PWMSetDuty_t pwm_set_duty;
    /* 总线舵机 */
    Servo_UartSend_t uart_send;
} Servo_Init_Config_s;

typedef struct
{
    uint8_t servo_id;
    float angle;
    uint16_t recv_angle;
    ServoType_e servo_type;
    Servo_PWMSetDuty_t pwm_set_duty;
    Servo_UartSend_t uart_send;
} ServoInstance;

/**
 * @brief 初始化舵机
 * @param inst   实例(调用者分配)
 * @param config 初始化配置
 */
void ServoInit(ServoInstance *inst, const Servo_Init_Config_s *config);

/**
 * @brief 设置舵机角度
 * @param servo 舵机实例
 * @param angle PWM舵机: 占空比(0~1); 总线舵机: 角度值
 */
void ServoSetAngle(ServoInstance *servo, float angle);

/**
 * @brief 解析总线舵机返回帧(10字节)
 * @param servo  舵机实例
 * @param rxbuf  接收缓冲区
 * @param len    接收长度
 */
void ServoDecodeFrame(ServoInstance *servo, const uint8_t *rxbuf, uint16_t len);

#endif
