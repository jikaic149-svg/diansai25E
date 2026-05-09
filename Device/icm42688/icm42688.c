/**
 * @file    icm42688.c
 * @brief   由纯函数指针接管的六轴控制器源码
 */
#include "icm42688.h"

int8_t ICM42688_ReadRegs(ICM42688_t *inst, uint8_t reg, uint8_t* buf, uint16_t len)
{
    if (!inst || !inst->CS_Control || !inst->SPI_ReadWrite) return -1;
    
    reg |= 0x80; // 最高位置 1 为读操作
    
    inst->CS_Control(0);
    inst->SPI_ReadWrite(reg);
    while (len--) {
        *buf = inst->SPI_ReadWrite(0x00);
        buf++;
    }
    inst->CS_Control(1);
    
    return 0;
}

int8_t ICM42688_WriteReg(ICM42688_t *inst, uint8_t reg, uint8_t value)
{
    if (!inst || !inst->CS_Control || !inst->SPI_ReadWrite) return -1;
    
    inst->CS_Control(0);
    inst->SPI_ReadWrite(reg);
    inst->SPI_ReadWrite(value);
    inst->CS_Control(1);
    
    return 0;
}

int8_t ICM42688_Init(ICM42688_t *inst)
{
    if (!inst) return -1;
    
    // 写复位: REG_BANK_SEL 软复位 (0x76, 0x01)
    ICM42688_WriteReg(inst, 0x76, 0x01); 
    if (inst->Delay_Ms) {
        inst->Delay_Ms(100); 
    }
    
    // 省略后续配置代码：用户只需按照需要将原厂其它写入翻译至此即可
    // 更新实例结构体的物理参数
    inst->accSensitivity = 2000.0f / 32768.0f;
    inst->gyroSensitivity = 1000.0f / 32768.0f;
    
    return 0;
}
