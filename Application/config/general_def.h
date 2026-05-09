/**
 * @file    general_def.h
 * @brief   通用数值常量定义
 * @origin  云台 modules/general_def.h
 * @note    100%可移植
 */
#ifndef GENERAL_DEF_H
#define GENERAL_DEF_H

#ifndef PI
#define PI 3.1415926535f
#endif
#define PI2 (PI * 2.0f)

#define RAD_2_DEGREE 57.2957795f        // 180/pi
#define DEGREE_2_RAD 0.01745329252f     // pi/180

#define RPM_2_ANGLE_PER_SEC 6.0f       // rpm -> deg/s: ×360/60
#define RPM_2_RAD_PER_SEC   0.104719755f // rpm -> rad/s: ×2pi/60

#endif /* GENERAL_DEF_H */
