/**
 * @file    vl53l1x_port.c
 * @brief   VL53L1X ST 标准库 ULD I2C 壳
 */
#include "vl53l1x_port.h"

void VL53L1X_Port_Init(VL53L1X_t *inst)
{
    if (!inst) return;
    
    /* 1. 若无特殊指定硬件重置，强制推入出厂 0x52 地址 */
    if (inst->DeviceAddress == 0) {
        inst->DeviceAddress = 0x52; 
    }
    
    /* 2. 此处配合使用 ST 官方的 VL53L1X ULD API 发送核心初始化命令...
       使用类似：
       inst->I2C_WriteReg(inst->DeviceAddress, REG_XXXX, &val, 1) 
       接管控即可。
     */
}
