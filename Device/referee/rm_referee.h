#ifndef RM_REFEREE_H
#define RM_REFEREE_H

/**
 * @file rm_referee.h
 * @brief 裁判系统数据接收与UI绘制支持（已移除BSP依赖）
 * @note  使用时需通过 RefereeSend 发送数据，用户需注册发送回调
 */

#include <stdint.h>
#include "referee_protocol.h"

extern uint8_t UI_Seq;

/* =================== 硬件抽象接口 =================== */

/**
 * @brief 裁判系统数据发送函数类型
 * @param data 待发送数据
 * @param len  数据长度
 */
typedef void (*Referee_SendFunc_t)(uint8_t *data, uint16_t len);

/**
 * @brief 裁判系统数据接收回调类型（接收完一帧后调用）
 * @param data 接收缓冲区
 * @param len  接收到的数据长度
 */
typedef void (*Referee_RxCallback_t)(uint8_t *data, uint16_t len);

#pragma pack(1)

typedef struct
{
	uint8_t Robot_Color;        // 机器人颜色 (Robot_Red=0, Robot_Blue=1)
	uint16_t Robot_ID;          // 本机器人ID
	uint16_t Cilent_ID;        // 本机器人对应的客户端ID
	uint16_t Receiver_Robot_ID; // 车间通信接收者ID
} referee_id_t;

// 裁判系统接收数据总结构
typedef struct
{
	referee_id_t referee_id;

	xFrameHeader FrameHeader;
	uint16_t CmdID;
	ext_game_state_t GameState;                            // 0x0001
	ext_game_result_t GameResult;                          // 0x0002
	ext_game_robot_HP_t GameRobotHP;                       // 0x0003
	ext_event_data_t EventData;                            // 0x0101
	ext_supply_projectile_action_t SupplyProjectileAction; // 0x0102
	ext_game_robot_state_t GameRobotState;                 // 0x0201
	ext_power_heat_data_t PowerHeatData;                   // 0x0202
	ext_game_robot_pos_t GameRobotPos;                     // 0x0203
	ext_buff_musk_t BuffMusk;                              // 0x0204
	aerial_robot_energy_t AerialRobotEnergy;               // 0x0205
	ext_robot_hurt_t RobotHurt;                            // 0x0206
	ext_shoot_data_t ShootData;                            // 0x0207

	Communicate_ReceiveData_t ReceiveData;

	uint8_t init_flag;

} referee_info_t;

// UI模式切换标志位
typedef struct
{
	uint32_t chassis_flag : 1;
	uint32_t gimbal_flag : 1;
	uint32_t shoot_flag : 1;
	uint32_t lid_flag : 1;
	uint32_t friction_flag : 1;
	uint32_t Power_flag : 1;
} Referee_Interactive_Flag_t;

// 底盘功率数据
typedef struct
{
	float chassis_power_mx;
} Chassis_Power_Data_s;

// UI绘制交互数据
typedef struct
{
	Referee_Interactive_Flag_t Referee_Interactive_Flag;

	uint8_t chassis_mode;
	uint8_t gimbal_mode;
	uint8_t shoot_mode;
	uint8_t friction_mode;
	uint8_t lid_mode;
	Chassis_Power_Data_s Chassis_Power_Data;

	uint8_t chassis_last_mode;
	uint8_t gimbal_last_mode;
	uint8_t shoot_last_mode;
	uint8_t friction_last_mode;
	uint8_t lid_last_mode;
	Chassis_Power_Data_s Chassis_last_Power_Data;

} Referee_Interactive_info_t;

#pragma pack()

/**
 * @brief 注册裁判系统发送函数
 * @param send_fn 发送函数指针
 */
void Referee_RegisterSendFunc(Referee_SendFunc_t send_fn);

/**
 * @brief 裁判系统数据发送接口
 * @param send 发送数据首地址
 * @param tx_len 发送长度
 */
void RefereeSend(uint8_t *send, uint16_t tx_len);

/**
 * @brief 裁判系统数据解析（在UART接收回调中调用）
 * @param rx_buf 接收到的原始数据
 * @param len    数据长度
 * @param referee_info 解析结果存放结构体
 */
void RefereeDataParse(uint8_t *rx_buf, uint16_t len, referee_info_t *referee_info);

#endif // RM_REFEREE_H
