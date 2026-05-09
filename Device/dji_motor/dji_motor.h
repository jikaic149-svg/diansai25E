#ifndef DJI_MOTOR_H
#define DJI_MOTOR_H

/**
 * @file dji_motor.h
 * @brief DJI智能电机驱动（M3508/M2006/GM6020，CAN抽象）
 * @origin 云台 modules/motor/DJImotor/dji_motor.h
 */

#include <stdint.h>
#include "../motor_def.h"

#define DJI_MOTOR_CNT 12
#define SPEED_SMOOTH_COEF  0.85f
#define CURRENT_SMOOTH_COEF 0.9f
#define ECD_ANGLE_COEF_DJI 0.043945f // 360/8192

/* DJI电机CAN反馈信息 */
typedef struct
{
    uint16_t last_ecd;
    uint16_t ecd;             // 0-8191
    float angle_single_round; // 单圈角度
    float speed_aps;          // 角速度 deg/s
    int16_t real_current;
    uint8_t temperature;

    float total_angle;
    int32_t total_round;
} DJI_Motor_Measure_s;

typedef struct
{
    DJI_Motor_Measure_s measure;
    Motor_Control_Setting_s motor_settings;
    Motor_Controller_s motor_controller;

    /* CAN配置 */
    uint32_t rx_id;
    uint8_t  can_bus;
    uint8_t  sender_group;
    uint8_t  message_num;

    Motor_Type_e motor_type;
    Motor_Working_Type_e stop_flag;

    uint32_t feed_cnt;
    float dt;
} DJIMotorInstance;

/**
 * @brief CAN发送单帧函数(全局注册一次)
 * @param tx_id   发送ID
 * @param data    8字节
 * @param can_bus 总线号(0/1/2)
 */
typedef void (*DJI_CANTransmit_t)(uint32_t tx_id, const uint8_t *data, uint8_t can_bus);

/**
 * @brief 获取时间增量函数（用于dt计算）
 * @param cnt 计数器指针
 * @return dt(秒)
 */
typedef float (*DJI_GetDeltaT_t)(uint32_t *cnt);

/**
 * @brief 注册CAN发送和计时函数
 */
void DJIMotorRegisterHW(DJI_CANTransmit_t can_send, DJI_GetDeltaT_t get_dt);

/**
 * @brief 注册DJI电机
 */
DJIMotorInstance *DJIMotorInit(Motor_Init_Config_s *config);

/**
 * @brief 设定电机参考值
 */
void DJIMotorSetRef(DJIMotorInstance *motor, float ref);

/**
 * @brief 切换反馈来源
 */
void DJIMotorChangeFeed(DJIMotorInstance *motor, Closeloop_Type_e loop, Feedback_Source_e type);

/**
 * @brief 停止电机
 */
void DJIMotorStop(DJIMotorInstance *motor);

/**
 * @brief 启动电机
 */
void DJIMotorEnable(DJIMotorInstance *motor);

/**
 * @brief 修改外层闭环
 */
void DJIMotorOuterLoop(DJIMotorInstance *motor, Closeloop_Type_e outer_loop);

/**
 * @brief 设置外部输出值（手动模式）
 */
void DJIMotorSetExternal(DJIMotorInstance *motor, float external);

/**
 * @brief CAN接收回调，由BSP层调用: DJIMotorDecode(rx_id, rx_data, can_bus)
 * @note  自动匹配注册过的电机实例
 */
void DJIMotorDecode(uint32_t rx_id, uint8_t *rx_data, uint8_t can_bus);

/**
 * @brief 计算所有电机PID并发送控制报文
 */
void DJIMotorControl(void);

#endif
