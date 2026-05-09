#ifndef __CONFIG_H
#define __CONFIG_H

// ==========================================
//  1. 机械与硬件基础参数 (基于 MG310 GMR版)
// ==========================================
#define WHEEL_DIAMETER 48.0f                 // 轮子直径 (单位: mm)
#define WHEEL_RADIUS (WHEEL_DIAMETER / 2.0f) // 轮子半径 (mm)
#define WHEEL_CIRCUMFERENCE                                                    \
  (3.14159265f * WHEEL_DIAMETER) // 轮子周长 (mm) ≈150.8mm
#define REDUCTION_RATIO 20.0f    // 电机减速比 1:20
#define ENCODER_LINES 13.0f      // 霍尔编码器线数
#define MULTIPLIER 4.0f          // 4 倍频解码
#define TOTAL_RESOLUTION (ENCODER_LINES * MULTIPLIER * REDUCTION_RATIO) // =1040
#define TRACK_WIDTH 128.0f // 左右轮中心距 (单位: mm)
#define WHEEL_BASE 113.0f  // 前后轮轴距 (单位: mm)
// 车辆几何参数 (单位: mm，以车身几何中心为原点 [0,0])（暂时）
#define CAR_LENGTH 210.0f      // 车身总长度 (如包含最前端红外探头到尾部的距离)
#define CENTER_TO_FRONT 100.0f // 几何中心到车头最前端的距离
#define CENTER_TO_REAR 90.0f   // 几何中心到车尾最后端的距离

// ==========================================
// 2. D24A 驱动板输出参数 (防烧车、防死区)
// ==========================================
#define PWM_MAX_LIMIT 999 // PWM 最大值
#define MOTOR_DEADZONE 0  // PID 闭环时由积分项处理死区，设为 0 禁用硬补偿

// ==========================================
// 4. 角度外环 PID 参数 (维持定海神针)
// ==========================================
#define PID_YAW_KP 8.0f  // 直线纠偏 P：快速响应
#define PID_YAW_KI 0.02f // 直线纠偏 I：极小积分消除稳态偏差
#define PID_YAW_KD 3.0f  // 直线纠偏 D：阻尼抑制振荡（配合 dt=1.0）

// Curve 模式：串级 PID（外环角度→目标角速度，内环角速度→差速 mm/s）
// 内环 200Hz (5ms), 外环 50Hz (每 4 个 inner tick)；增益保守，实车标定
#define PID_YAW_OUTER_KP        2.0f    // 外环：航向误差(°) → 目标 Wz(°/s) 比例
#define PID_YAW_OUTER_KI        0.0f    // 外环积分默认关闭
#define PID_YAW_OUTER_MAX_WZ    100.0f  // 目标 Wz 限幅 (°/s)，避免目标过激
#define PID_WZ_INNER_KP         2.5f    // 内环：Wz 误差(°/s) → 差速(mm/s) 比例
#define PID_WZ_INNER_KI         8.0f    // 内环积分消除稳态偏差
#define PID_WZ_INNER_INT_LIMIT  150.0f  // 内环积分限幅
#define PID_WZ_INNER_MAX_DIFF   130.0f  // 差速限幅 (mm/s)，降低至 130 避免右轮突破 400 导致抽搐

// 循迹模式微调 (配合极性归一化后的质心误差 ±5.0)
// 要求极致平稳：适度降低 P（避免左右横跳），显著提高 D（增强阻尼，消除震荡）
#define PID_TRACK_KP 15.0f        // P值：15 * 5(最大误差) = 75 轮速差
#define PID_TRACK_KI 0.0f         // I值：关闭防过冲
#define PID_TRACK_KD 45.0f        // D值：提供极强阻尼，抑制任何突变
#define PID_TRACK_WZ_KD 0.8f      // YAW阻尼(陀螺仪Z轴角速度)：显著抑制车身摇摆
#define TRACK_MAX_TURN_RATIO 0.7f // 转向限幅收紧至基速 70%，拒绝急转

// ==========================================
// 6. 矩形循迹任务参数 (TASK_SQUARE)
// ==========================================
#define SQUARE_BASE_SPEED         200.0f   // 恒定基速降至 200 (mm/s)，针对 20s 完赛时间重新测算
#define SQUARE_RAMP_ACCEL         200.0f   // 起步/停车线性斜坡 (mm/s^2)，1s 内平滑达到最高速
#define SQUARE_TURN_RADIUS        150.0f   // 内切圆半径 (mm)
#define SQUARE_TURN_ANTICIPATE    805.0f   // 基于 200mm/s 的 Euler spiral 积分，提前 195mm 入弯
#define SQUARE_LAP_LEN            1000.0f  // 单边长度 (mm)，1m
#define SQUARE_TOTAL_CORNERS      4        // 4 个直角后停车
#define SQUARE_LOST_LINE_TIMEOUT  5000     // 丢线 5 秒强制制动 (ms)
#define SQUARE_TURN_DONE_DEG      2.0f     // 转弯完成阈值 (°)

// ==========================================
// 7. 模式层转向与速度参数 (轮速闭环, 单位: mm/s)
// ==========================================
#define STRAIGHT_TURN_RATIO 0.4f            // 直线模式：纠偏占基速的 40%
#define MODE_STRAIGHT_MAX_CORRECTION 400.0f // 直线模式最大航向纠偏 (mm/s)
#define AXIS_MAX_TURN_SPEED 150.0f          // 轴向模式最大转向速度 (mm/s)
#define AXIS_BASE_SPEED 250.0f              // 轴向模式基础前进速度 (mm/s)
#define AXIS_MIN_SPEED 50.0f                // 轴向模式最小前进速度 (mm/s)
#define AXIS_TURN_RATIO 0.7f                // 轴向模式：转向占基速的 70%
#define TURNING_MIN_SPEED 150.0f // 转向模式：克服静摩擦最小轮速
#define TURNING_MAX_SPEED 350.0f // 转向模式：最大轮速
#define PID_TURNING_KD 18.0f      // 转向 D：强阻尼减少过冲振荡
#define TURNING_DECEL_ZONE 15.0f // 转向减速区

// ==========================================
// 9. 轮速 PID 闭环参数 (目标: mm/s, 输出: PWM)
//    适配范围 450~700 mm/s
// ==========================================
#define PID_WHEEL_KP 3.5f               // 速度环 P：加快初始响应
#define PID_WHEEL_KI 5.0f               // 速度环 I：大幅加速稳态收敛
#define PID_WHEEL_KD 0.005f             // 速度环 D：降低抑制超调
#define PID_WHEEL_MAX_OUT 999.0f        // 输出限幅 = PWM 最大值
#define PID_WHEEL_INTEGRAL_LIMIT 500.0f // 积分限幅 (防饱和)
#define PID_WHEEL_DEADBAND 3.0f         // 死区 (mm/s)
#define PID_WHEEL_FF 0.85f              // 前馈增益 (降低防起步超调)

// ==========================================
// 10. 制动参数
// ==========================================
#define BRAKE_SPEED_THRESHOLD 20.0f      // 停稳判定速度阈值 (mm/s)
#define BRAKE_STABLE_COUNT 3             // 连续低速 3 次视为停稳

#endif
