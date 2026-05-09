/**
 * @file    robot_def.h
 * @brief   机器人参数定义模板 — 根据实际机器人修改下方宏定义
 * @origin  云台 application/robot_def.h
 * @note    本文件定义了机器人的所有枚举类型和控制数据结构
 *          迁移到新项目时, 只需修改 "机器人参数" 区域的宏
 */
#ifndef ROBOT_DEF_H
#define ROBOT_DEF_H

#include <stdint.h>

/* ======================== 开发板类型(仅保留一个!) ======================== */
#define ONE_BOARD           // 单板控制整车
// #define CHASSIS_BOARD    // 底盘板
// #define GIMBAL_BOARD     // 云台板

/* ======================== 机器人参数(按实际修改) ======================== */
// 云台参数
#define YAW_CHASSIS_ALIGN_ECD   2711    // 云台与底盘对齐时电机编码器值
#define PITCH_HORIZON_ECD       3412    // 云台水平时的编码器值
#define PITCH_MAX_ANGLE         0       // 云台最大俯仰角
#define PITCH_MIN_ANGLE         0       // 云台最小俯仰角

// 底盘参数(mm)
#define WHEEL_BASE              350     // 纵向轴距
#define TRACK_WIDTH             300     // 横向轮距
#define CENTER_GIMBAL_OFFSET_X  0       // 云台中心X偏移
#define CENTER_GIMBAL_OFFSET_Y  0       // 云台中心Y偏移
#define RADIUS_WHEEL            60      // 轮子半径
#define REDUCTION_RATIO_WHEEL   19.0f   // 电机减速比

// 方向映射
#define GYRO2GIMBAL_DIR_YAW     1       // 陀螺仪vs云台yaw方向(1同/-1反)
#define GYRO2GIMBAL_DIR_PITCH   1
#define GYRO2GIMBAL_DIR_ROLL    1

/* ======================== 冲突检测 ======================== */
#if (defined(ONE_BOARD) && defined(CHASSIS_BOARD)) || \
    (defined(ONE_BOARD) && defined(GIMBAL_BOARD)) ||  \
    (defined(CHASSIS_BOARD) && defined(GIMBAL_BOARD))
#error Conflict board definition! You can only define one board type.
#endif

/* ======================== 基本控制模式枚举 ======================== */
typedef enum { ROBOT_STOP = 0, ROBOT_READY } Robot_Status_e;
typedef enum { APP_OFFLINE = 0, APP_ONLINE, APP_ERROR } App_Status_e;

typedef enum
{
    CHASSIS_ZERO_FORCE = 0,
    CHASSIS_ROTATE,
    CHASSIS_NO_FOLLOW,
    CHASSIS_FOLLOW_GIMBAL_YAW,
} chassis_mode_e;

typedef enum
{
    GIMBAL_ZERO_FORCE = 0,
    GIMBAL_FREE_MODE,
    GIMBAL_GYRO_MODE,
} gimbal_mode_e;

typedef enum { SHOOT_OFF = 0, SHOOT_ON } shoot_mode_e;
typedef enum { FRICTION_OFF = 0, FRICTION_ON } friction_mode_e;
typedef enum { LID_OPEN = 0, LID_CLOSE } lid_mode_e;
typedef enum { LOAD_STOP = 0, LOAD_REVERSE, LOAD_1_BULLET, LOAD_3_BULLET, LOAD_BURSTFIRE } loader_mode_e;

/* ======================== 控制数据结构 ======================== */
#pragma pack(1)

typedef struct
{
    float vx;
    float vy;
    float wz;
    float offset_angle;
    chassis_mode_e chassis_mode;
    int chassis_speed_buff;
} Chassis_Ctrl_Cmd_s;

typedef struct
{
    float yaw;
    float pitch;
    float chassis_rotate_wz;
    gimbal_mode_e gimbal_mode;
} Gimbal_Ctrl_Cmd_s;

typedef struct
{
    shoot_mode_e shoot_mode;
    loader_mode_e load_mode;
    lid_mode_e lid_mode;
    friction_mode_e friction_mode;
    uint8_t rest_heat;
    float shoot_rate;
} Shoot_Ctrl_Cmd_s;

#pragma pack()

#endif /* ROBOT_DEF_H */
