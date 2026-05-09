/**
 * @file servo_motor.c
 * @brief 舵机驱动实现
 */

#include "servo_motor.h"
#include <string.h>

/* 总线舵机命令帧 */
static uint8_t servo_angle_write[16] = {
    0x55, 0x55, 0x08, 0x03, 0x01, 0xF4, 0x01, 0x01, 0x20, 0x03,
    0x55, 0x55, 0x04, 0x15, 0x01, 0x01
};

void ServoInit(ServoInstance *inst, const Servo_Init_Config_s *config)
{
    memset(inst, 0, sizeof(ServoInstance));
    inst->servo_type = config->servo_type;
    inst->servo_id   = config->servo_id;

    if (config->servo_type == PWM_Servo)
        inst->pwm_set_duty = config->pwm_set_duty;
    else if (config->servo_type == Bus_Servo)
        inst->uart_send = config->uart_send;
}

void ServoSetAngle(ServoInstance *servo, float angle)
{
    switch (servo->servo_type)
    {
    case Bus_Servo:
        if (!servo->uart_send) return;
        servo_angle_write[8] = (uint16_t)angle & 0xff;
        servo_angle_write[9] = (uint16_t)angle >> 8;
        servo->uart_send(servo_angle_write, 16);
        break;
    case PWM_Servo:
        if (!servo->pwm_set_duty) return;
        servo->angle = angle;
        servo->pwm_set_duty(angle);
        break;
    default:
        break;
    }
}

void ServoDecodeFrame(ServoInstance *servo, const uint8_t *rxbuf, uint16_t len)
{
    if (len < 8) return;
    if (servo->servo_type != Bus_Servo) return;

    if (rxbuf[0] == Servo_Frame_First && rxbuf[1] == Servo_Frame_Second)
    {
        if (rxbuf[3] == SERVO_POS_READ_CMD)
        {
            servo->recv_angle = (uint16_t)(rxbuf[7] << 8 | rxbuf[6]);
        }
    }
}
