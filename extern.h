/**
 * @file extern.h
 * @brief 系统全局公共实例统一声明 (Rule 6: 集中管理)
 */
#ifndef EXTERN_H
#define EXTERN_H

#include "Device/motor/motor.h"
#include "Device/jy61p/jy61p.h"
#include "Device/motor_encoder/motor_encoder.h"
#include "Application/algorithm/inertial_nav.h"
#include "Device/track/track.h"

extern Motor_t g_motor_left;
extern Motor_t g_motor_right;
extern MotorEncoder_t g_enc_left;
extern MotorEncoder_t g_enc_right;
extern JY61P_Instance g_jy61p;
extern INS_Instance g_ins;
extern Track_Instance g_track;

#endif /* EXTERN_H */
