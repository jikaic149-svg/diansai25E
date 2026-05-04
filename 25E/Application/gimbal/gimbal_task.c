#include "gimbal_task.h"
#include "FreeRTOS.h"
#include "task.h"
#include "config.h"
#include "bsp_can.h"
#include "ins_task.h"  // 用于获取 global_gimbal_pitch 等全局变量

/* ======================== 独立的云台控制任务 ======================== */

void Gimbal_Task(void *argument)
{
    (void)argument;
    
    // 任务频率：1000Hz (每毫秒唤醒一次)
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(1); 
    
    // PID 变量
    float target_pitch = 0.0f;
    float target_yaw   = 0.0f;
    float pitch_i_out  = 0.0f;
    float yaw_i_out    = 0.0f;
    uint8_t pid_init_flag = 0;
    
    // === D 环低通滤波器历史值 ===
    static float filtered_pitch_speed = 0.0f;
    static float filtered_yaw_speed   = 0.0f;
    
    // 等待 IMU 和 INS 校准完成
    extern uint8_t is_zeroed; 
    while (!is_zeroed) {
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
    for (;;)
    {
        // === 姿态锁定：上电校准完成后的那一瞬间，记录当前姿态作为目标 ===
        if (pid_init_flag == 0) {
            target_pitch = global_gimbal_pitch;
            target_yaw   = global_gimbal_yaw;
            pid_init_flag = 1;
        }
        
        // 读取由 INS_Task 解算好的全局姿态
        float current_pitch = global_gimbal_pitch;
        float current_yaw   = global_gimbal_yaw;
        
        // === PD 误差计算 ===
        float err_pitch = target_pitch - current_pitch;
        float err_yaw   = target_yaw - current_yaw;
        
        // Yaw 环防缠绕处理 (走最短路径)
        if (err_yaw > 180.0f) err_yaw -= 360.0f;
        if (err_yaw < -180.0f) err_yaw += 360.0f;
        
        // Pitch 环防缠绕/防死锁处理 (防止Pitch跨越90度或180度时产生极大误差)
        if (err_pitch > 180.0f) err_pitch -= 360.0f;
        if (err_pitch < -180.0f) err_pitch += 360.0f;
        
        // === 极度安全的角度误差限幅 (防大扰动画龙和疯转的核心) ===
        // 限制 P 环输入误差，不管外力多大，系统最多认为自己偏了 MAX_ERR 度。
        if (err_pitch > GIMBAL_MAX_ERR_PITCH) err_pitch = GIMBAL_MAX_ERR_PITCH;
        if (err_pitch < -GIMBAL_MAX_ERR_PITCH) err_pitch = -GIMBAL_MAX_ERR_PITCH;
        
        if (err_yaw > GIMBAL_MAX_ERR_YAW) err_yaw = GIMBAL_MAX_ERR_YAW;
        if (err_yaw < -GIMBAL_MAX_ERR_YAW) err_yaw = -GIMBAL_MAX_ERR_YAW;
        
        // 保存原始误差用于积分项
        float original_err_pitch = err_pitch;
        float original_err_yaw   = err_yaw;
        
        // === 引入控制死区 (Deadband) 平滑处理 ===
        // 直接清零会导致出死区时突变引发剧烈震荡，正确的做法是误差平移
        if (err_pitch > GIMBAL_PITCH_DEADBAND) {
            err_pitch -= GIMBAL_PITCH_DEADBAND;
        } else if (err_pitch < -GIMBAL_PITCH_DEADBAND) {
            err_pitch += GIMBAL_PITCH_DEADBAND;
        } else {
            err_pitch = 0.0f;
        }
        
        if (err_yaw > GIMBAL_YAW_DEADBAND) {
            err_yaw -= GIMBAL_YAW_DEADBAND;
        } else if (err_yaw < -GIMBAL_YAW_DEADBAND) {
            err_yaw += GIMBAL_YAW_DEADBAND;
        } else {
            err_yaw = 0.0f;
        }
        
        // === 积分项计算与条件清零 (积分分离，极其重要！) ===
        // 如果遇到大的外部扰动（误差大于3度），必须【强行清零】积分！
        // 否则你用手强行掰住云台几秒，积分项会疯狂蓄力爆满。当你一松手，
        // 庞大的积分残余力会带着云台严重冲过目标点（积分饱和引起的超调）。
        if (original_err_pitch > 3.0f || original_err_pitch < -3.0f) {
            pitch_i_out = 0.0f;  // 误差大，直接清空，全靠 P 环拉回
        } else {
            pitch_i_out += GIMBAL_PID_PITCH_KI * original_err_pitch; // 误差小（快到 0 点了），才启动积分慢慢对正
        }
        
        if (original_err_yaw > 3.0f || original_err_yaw < -3.0f) {
            yaw_i_out = 0.0f;
        } else {
            yaw_i_out += GIMBAL_PID_YAW_KI * original_err_yaw;
        }
        
        // 积分基础限幅 (双重保险)
        if (pitch_i_out > GIMBAL_MAX_I_PITCH) pitch_i_out = GIMBAL_MAX_I_PITCH;
        if (pitch_i_out < -GIMBAL_MAX_I_PITCH) pitch_i_out = -GIMBAL_MAX_I_PITCH;
        if (yaw_i_out > GIMBAL_MAX_I_YAW) yaw_i_out = GIMBAL_MAX_I_YAW;
        if (yaw_i_out < -GIMBAL_MAX_I_YAW) yaw_i_out = -GIMBAL_MAX_I_YAW;
        
        // === 真正的 D 环阻尼（低通滤波，极度重要！） ===
        // 直接使用原始角速度计算 KD 会将传感器的高频噪声无限放大，导致电机“高频尖叫振荡”
        // 采用一阶低通滤波 (EMA) 让速度曲线变得极度丝滑，彻底消除恶心的嗡嗡声。
        // 滤波系数 0.7(旧) + 0.3(新)，在平滑度和延迟之间取最佳平衡。
        filtered_pitch_speed = 0.3f * filtered_pitch_speed + 0.7f * global_imu_pitch_speed;
        filtered_yaw_speed   = 0.3f * filtered_yaw_speed   + 0.7f * global_imu_yaw_speed;
        
        // === 电流输出计算 (P + I + D) ===
        float out_pitch_cur = (GIMBAL_PID_PITCH_KP * err_pitch + pitch_i_out - GIMBAL_PID_PITCH_KD * filtered_pitch_speed) * PITCH_POLARITY;
        float out_yaw_cur   = (GIMBAL_PID_YAW_KP * err_yaw + yaw_i_out - GIMBAL_PID_YAW_KD * filtered_yaw_speed) * YAW_POLARITY;
        
        // === 极度安全的限幅 (防击穿！绝对不可移除！) ===
        if (out_pitch_cur > GIMBAL_MAX_OUTPUT_CURRENT) out_pitch_cur = GIMBAL_MAX_OUTPUT_CURRENT;
        if (out_pitch_cur < -GIMBAL_MAX_OUTPUT_CURRENT) out_pitch_cur = -GIMBAL_MAX_OUTPUT_CURRENT;
        if (out_yaw_cur > GIMBAL_MAX_OUTPUT_CURRENT) out_yaw_cur = GIMBAL_MAX_OUTPUT_CURRENT;
        if (out_yaw_cur < -GIMBAL_MAX_OUTPUT_CURRENT) out_yaw_cur = -GIMBAL_MAX_OUTPUT_CURRENT;
        
        // 急停保护：若 PA0 被拉低，软清零电流而不失能电机
        if (emergency_stop) {
            out_pitch_cur = 0.0f;
            out_yaw_cur   = 0.0f;
            pitch_i_out   = 0.0f; // 同时清空积分
            yaw_i_out     = 0.0f;
        }
        
        // === 下发电流控制电机 ===
        Motor_SetCurrent(0x01, 0.0f);        // Pitch 暂时关闭，只调 Yaw
        Motor_SetCurrent(0x00, out_yaw_cur);   // 发送给 Yaw
        
        // 挂起任务直到下一毫秒 (1000Hz 稳定调度)
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}
