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
    uint8_t pid_init_flag = 0;
    
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
        
        // === 极度安全的角度误差限幅 (用户要求) ===
        
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
        
        // === 真正的 D 环阻尼（取消人工负号） ===
        // 去除了高频裸差分后，我们恢复使用硬件平滑的 speed。
        // 上一次加了负号导致了严重的“画大圆”发散（刹车变成了超级油门），说明硬件底层传来的极性本来就是对的！
        float real_pitch_speed = global_imu_pitch_speed;
        float real_yaw_speed   = global_imu_yaw_speed;
        
        // === 电流输出计算 ===
        float out_pitch_cur = (GIMBAL_PID_PITCH_KP * err_pitch - GIMBAL_PID_PITCH_KD * real_pitch_speed) * PITCH_POLARITY;
        float out_yaw_cur   = (GIMBAL_PID_YAW_KP * err_yaw - GIMBAL_PID_YAW_KD * real_yaw_speed) * YAW_POLARITY;
        
        // === 极度安全的限幅 (防击穿！绝对不可移除！) ===
        if (out_pitch_cur > GIMBAL_MAX_OUTPUT_CURRENT) out_pitch_cur = GIMBAL_MAX_OUTPUT_CURRENT;
        if (out_pitch_cur < -GIMBAL_MAX_OUTPUT_CURRENT) out_pitch_cur = -GIMBAL_MAX_OUTPUT_CURRENT;
        if (out_yaw_cur > GIMBAL_MAX_OUTPUT_CURRENT) out_yaw_cur = GIMBAL_MAX_OUTPUT_CURRENT;
        if (out_yaw_cur < -GIMBAL_MAX_OUTPUT_CURRENT) out_yaw_cur = -GIMBAL_MAX_OUTPUT_CURRENT;
        
        // 急停保护：若 PA0 被拉低，软清零电流而不失能电机
        if (emergency_stop) {
            out_pitch_cur = 0.0f;
            out_yaw_cur   = 0.0f;
        }
        
        // === 下发电流控制电机 ===
        Motor_SetCurrent(0x01, out_pitch_cur); // 发送给 Pitch
        Motor_SetCurrent(0x00, out_yaw_cur);   // 发送给 Yaw
        
        // 挂起任务直到下一毫秒 (1000Hz 稳定调度)
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}
