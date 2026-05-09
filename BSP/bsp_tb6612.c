/**
 * @file bsp_tb6612.c
 * @brief BSP TB6612 电机驱动 GPIO 控制实现
 */
#include "bsp_tb6612.h"
#include "ti_msp_dl_config.h"

void BSP_TB6612_SetEnable(bool enable)
{
    if (enable) {
        DL_GPIO_setPins(GPIO_MOTOR_DIR_STBY_PORT, GPIO_MOTOR_DIR_STBY_PIN);
    } else {
        DL_GPIO_clearPins(GPIO_MOTOR_DIR_STBY_PORT, GPIO_MOTOR_DIR_STBY_PIN);
    }
}

void BSP_TB6612_SetDirLeft(int8_t dir)
{
    if (dir > 0) {
        DL_GPIO_setPins(GPIO_MOTOR_DIR_AIN1_PORT, GPIO_MOTOR_DIR_AIN1_PIN);
        DL_GPIO_clearPins(GPIO_MOTOR_DIR_AIN2_PORT, GPIO_MOTOR_DIR_AIN2_PIN);
    } else if (dir < 0) {
        DL_GPIO_clearPins(GPIO_MOTOR_DIR_AIN1_PORT, GPIO_MOTOR_DIR_AIN1_PIN);
        DL_GPIO_setPins(GPIO_MOTOR_DIR_AIN2_PORT, GPIO_MOTOR_DIR_AIN2_PIN);
    } else {
        DL_GPIO_clearPins(GPIO_MOTOR_DIR_AIN1_PORT, GPIO_MOTOR_DIR_AIN1_PIN);
        DL_GPIO_clearPins(GPIO_MOTOR_DIR_AIN2_PORT, GPIO_MOTOR_DIR_AIN2_PIN);
    }
}

void BSP_TB6612_SetDirRight(int8_t dir)
{
    /* 右电机物理镜像安装，方向反转：正转→BIN2高/BIN1低 */
    if (dir > 0) {
        DL_GPIO_clearPins(GPIO_MOTOR_DIR_BIN1_PORT, GPIO_MOTOR_DIR_BIN1_PIN);
        DL_GPIO_setPins(GPIO_MOTOR_DIR_BIN2_PORT, GPIO_MOTOR_DIR_BIN2_PIN);
    } else if (dir < 0) {
        DL_GPIO_setPins(GPIO_MOTOR_DIR_BIN1_PORT, GPIO_MOTOR_DIR_BIN1_PIN);
        DL_GPIO_clearPins(GPIO_MOTOR_DIR_BIN2_PORT, GPIO_MOTOR_DIR_BIN2_PIN);
    } else {
        DL_GPIO_clearPins(GPIO_MOTOR_DIR_BIN1_PORT, GPIO_MOTOR_DIR_BIN1_PIN);
        DL_GPIO_clearPins(GPIO_MOTOR_DIR_BIN2_PORT, GPIO_MOTOR_DIR_BIN2_PIN);
    }
}
