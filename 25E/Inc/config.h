#ifndef __CONFIG_H__
#define __CONFIG_H__

/*
 * ========================================================
 * 强动力 (QD) 电机参数配置 (Motor Parameter Config)
 * 根据您提供的图片参数：
 * ========================================================
 */

#define MOTOR_RATED_VOLTAGE_MIN  7     // 额定电压最小值 (V)
#define MOTOR_RATED_VOLTAGE_MAX  26    // 额定电压最大值 (V)
#define MOTOR_RATED_CURRENT      1.0f  // 额定电流 (A)
#define MOTOR_PEAK_CURRENT       2.0f  // 峰值电流 (A)
#define MOTOR_RATED_TORQUE       0.2f  // 额定扭矩 (Nm)
#define MOTOR_PEAK_TORQUE        0.3f  // 峰值扭矩 (Nm)
#define MOTOR_RATED_SPEED        500   // 额定转速 (rpm)
#define MOTOR_PEAK_SPEED         800   // 峰值转速 (rpm)
#define MOTOR_POLE_PAIRS         14    // 极对数
#define MOTOR_ENCODER_BITS       15    // 编码器位数 (bit)
#define MOTOR_CONTROL_ACCURACY   0.0109f // 控制精度 (°)

/*
 * 通信方式: CAN, USB, UART
 * CAN 波特率: 1 Mbps
 * UART 波特率: 115200 bps
 * 控制模式: 电流模式、速度模式、角度模式、低速大力矩模式、角度步进模式
 */

/*
 * --------------------------------------------------------
 * 电机内部控制环参数与限制 (导出的 config)
 * --------------------------------------------------------
 */
#define MOTOR_PID_SPEED_KP       0.00300f
#define MOTOR_PID_SPEED_KI       0.000390f
#define MOTOR_PID_SPEED_KD       0.000f

#define MOTOR_PID_ANGLE_KP       1200.0f
#define MOTOR_PID_ANGLE_KI       0.000f
#define MOTOR_PID_ANGLE_KD       0.000f

#define MOTOR_LIMIT_SPEED        1000.0f  // 速度限制 (rpm)
#define MOTOR_LIMIT_CURRENT      1.65f    // 电流限制 (A)

/*
 * --------------------------------------------------------
 * 机械结构参数 (校准后填入)
 * --------------------------------------------------------
 */
#define MOTOR_PITCH_MECH_ZERO    0.0f     // Pitch 轴机械零点 (单位: 度)
#define MOTOR_YAW_MECH_ZERO      0.0f     // Yaw 轴机械零点 (单位: 度)

/*
 * --------------------------------------------------------
 * 云台外环 (1000Hz) PID 增稳控制参数 (极度保守)
 * --------------------------------------------------------
 */
// 极性宏：极其重要！如果发生正反馈震荡（一碰就加速甩头），请将其改为 -1.0f
#define PITCH_POLARITY 1.0f
#define YAW_POLARITY   -1.0f

// 针对云台惯量调整：加大 YAW 力度，减小 PITCH 力度
// 注意：一旦方向正确，系统仍然会画大圆发散的原因是缺少阻尼（D环）！
// 现在重新引入并增强 D 环，将其稳死。
#define GIMBAL_PID_PITCH_KP 0.025f
#define GIMBAL_PID_PITCH_KI 0.0001f
#define GIMBAL_PID_PITCH_KD 0.0020f
#define GIMBAL_PID_YAW_KP   0.050f
#define GIMBAL_PID_YAW_KI   0.0002f
#define GIMBAL_PID_YAW_KD   0.0010f

// 控制死区 (度)，在此误差范围内视为已到达目标，不进行 P 环修正，消除微抖动
#define GIMBAL_PITCH_DEADBAND 0.5f
#define GIMBAL_YAW_DEADBAND   0.2f

// === 核心保护：误差限幅 (极度重要，防画龙防疯转) ===
// 无论外面怎么扰动，系统认为的“最大误差”不超过此值。防止放手瞬间积累极大动能。
#define GIMBAL_MAX_ERR_PITCH  20.0f
#define GIMBAL_MAX_ERR_YAW    20.0f

// 积分项限幅 (防积分饱和)
#define GIMBAL_MAX_I_PITCH    0.3f  // 积分最大只允许提供 0.3A 电流，只用于消除微小稳态误差
#define GIMBAL_MAX_I_YAW      0.3f

// 安全限幅保护 (最大输出指令电流 A，用手能轻松捏住，防烧毁防击穿)
#define GIMBAL_MAX_OUTPUT_CURRENT 1.0f

#endif /* __CONFIG_H__ */
