/**
 * @file broker.h
 * @brief Application层：基于多话题分发的轻量级消息总线机制 (Pub/Sub)
 */
#ifndef APP_BROKER_H
#define APP_BROKER_H

#include <stdint.h>

/* 定义系统支持的所有话题 */
typedef enum {
    TOPIC_TARGET_SPEED_LEFT = 0,
    TOPIC_TARGET_SPEED_RIGHT,

    /* 惯性导航相关话题 */
    TOPIC_JY61P_YAW,
    TOPIC_JY61P_AX,
    TOPIC_JY61P_WZ,         /* Z 轴角速度 (°/s)，串级 PID 内环反馈 */
    TOPIC_INS_SPEED_X,
    TOPIC_INS_MILEAGE_X,

    /* Mode 状态机相关话题 */
    TOPIC_CAR_MODE,         /* 当前运行模式 (CarMode_e) */
    TOPIC_TRACK_ERROR,      /* 循迹传感器误差 (float) */

    /* Task 层话题 */
    TOPIC_TASK_ID,          /* 当前执行的任务编号 (0=无, 1~4) */

    /* 编码器实测速度 (mm/s) */
    TOPIC_SPEED_LEFT,       /* 左轮实测线速度 */
    TOPIC_SPEED_RIGHT,      /* 右轮实测线速度 */

    /* 速度闭环目标轮速 (mm/s) */
    TOPIC_TARGET_WHEEL_SPEED_LEFT,
    TOPIC_TARGET_WHEEL_SPEED_RIGHT,

    /* 编码器累积里程 (mm) */
    TOPIC_DISTANCE_LEFT,
    TOPIC_DISTANCE_RIGHT,

    TOPIC_MAX_NUM
} TopicID_e;

/* 初始化总线内存数据空间 */
void Broker_Init(void);

/* 基础浮点型深拷贝收发 (32bit 无锁访问) */
void  Broker_Publish_Float(TopicID_e topic, float value);
float Broker_Subscribe_Float(TopicID_e topic);

/* 基础整型深拷贝收发 (32bit 无锁访问) */
void    Broker_Publish_Int(TopicID_e topic, int32_t value);
int32_t Broker_Subscribe_Int(TopicID_e topic);

#endif /* APP_BROKER_H */
