/**
 * @file rm_referee.c
 * @brief 裁判系统数据收发（已移除BSP依赖，可移植）
 */

#include "rm_referee.h"
#include "crc_ref.h"
#include <string.h>

uint8_t UI_Seq = 0;

static Referee_SendFunc_t referee_send_fn = NULL;

void Referee_RegisterSendFunc(Referee_SendFunc_t send_fn)
{
    referee_send_fn = send_fn;
}

void RefereeSend(uint8_t *send, uint16_t tx_len)
{
    if (referee_send_fn != NULL)
    {
        referee_send_fn(send, tx_len);
    }
}

/**
 * @brief 裁判系统数据帧解析
 */
void RefereeDataParse(uint8_t *rx_buf, uint16_t len, referee_info_t *referee_info)
{
    uint16_t index = 0;

    while (index < len)
    {
        // 寻找帧头SOF
        if (rx_buf[index] != REFEREE_SOF)
        {
            index++;
            continue;
        }

        // 解析帧头
        xFrameHeader *header = (xFrameHeader *)&rx_buf[index];

        // CRC8校验帧头
        if (!Verify_CRC8_Check_Sum(&rx_buf[index], LEN_CRC8 + 1))
        {
            index++;
            continue;
        }

        uint16_t data_length = header->DataLength;
        uint16_t frame_length = LEN_HEADER + LEN_CMDID + data_length + LEN_TAIL;

        // 检查帧长度
        if (index + frame_length > len)
            break;

        // CRC16校验整帧
        if (!Verify_CRC16_Check_Sum(&rx_buf[index], frame_length))
        {
            index++;
            continue;
        }

        // 获取命令码
        uint16_t cmd_id = (rx_buf[index + 6] << 8) | rx_buf[index + 5];
        uint8_t *data_addr = &rx_buf[index + LEN_HEADER + LEN_CMDID];

        referee_info->CmdID = cmd_id;

        switch (cmd_id)
        {
        case ID_game_state:
            memcpy(&referee_info->GameState, data_addr, sizeof(ext_game_state_t));
            break;
        case ID_game_result:
            memcpy(&referee_info->GameResult, data_addr, sizeof(ext_game_result_t));
            break;
        case ID_game_robot_survivors:
            memcpy(&referee_info->GameRobotHP, data_addr, sizeof(ext_game_robot_HP_t));
            break;
        case ID_event_data:
            memcpy(&referee_info->EventData, data_addr, sizeof(ext_event_data_t));
            break;
        case ID_supply_projectile_action:
            memcpy(&referee_info->SupplyProjectileAction, data_addr, sizeof(ext_supply_projectile_action_t));
            break;
        case ID_game_robot_state:
            memcpy(&referee_info->GameRobotState, data_addr, sizeof(ext_game_robot_state_t));
            break;
        case ID_power_heat_data:
            memcpy(&referee_info->PowerHeatData, data_addr, sizeof(ext_power_heat_data_t));
            break;
        case ID_game_robot_pos:
            memcpy(&referee_info->GameRobotPos, data_addr, sizeof(ext_game_robot_pos_t));
            break;
        case ID_buff_musk:
            memcpy(&referee_info->BuffMusk, data_addr, sizeof(ext_buff_musk_t));
            break;
        case ID_aerial_robot_energy:
            memcpy(&referee_info->AerialRobotEnergy, data_addr, sizeof(aerial_robot_energy_t));
            break;
        case ID_robot_hurt:
            memcpy(&referee_info->RobotHurt, data_addr, sizeof(ext_robot_hurt_t));
            break;
        case ID_shoot_data:
            memcpy(&referee_info->ShootData, data_addr, sizeof(ext_shoot_data_t));
            break;
        case ID_student_interactive:
            memcpy(&referee_info->ReceiveData, data_addr, sizeof(Communicate_ReceiveData_t));
            break;
        default:
            break;
        }

        index += frame_length;
    }
}
