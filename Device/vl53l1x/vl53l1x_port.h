/**
 * @file    vl53l1x_port.h
 * @brief   VL53L1X 纯 C 平台移植代理框架 (彻底脱离 Arduino 以及 TwoWire C++ 生态)
 */
#ifndef VL53L1X_PORT_H
#define VL53L1X_PORT_H

#include <stdint.h>

/**
 * @brief 提供硬件下层实际拉拽 SCL / SDA 的 I2C 搬运机能
 */
typedef int8_t (*VL53L1X_I2C_WriteReg_Func_t)(uint16_t dev_addr, uint16_t reg_addr, uint8_t *data, uint16_t len);
typedef int8_t (*VL53L1X_I2C_ReadReg_Func_t)(uint16_t dev_addr, uint16_t reg_addr, uint8_t *data, uint16_t len);

/**
 * @brief 每一个测距探头实例
 *        这使得即便接了 8 个激光雷达，你也无需重写 8 遍代码，通过分地址或开路控制传入相同指针
 */
typedef struct {
    VL53L1X_I2C_WriteReg_Func_t I2C_WriteReg;
    VL53L1X_I2C_ReadReg_Func_t  I2C_ReadReg;
    uint16_t                    DeviceAddress; // 默认 0x52
} VL53L1X_t;

/* ======= 接口与宏代理 ======= */
void VL53L1X_Port_Init(VL53L1X_t *inst);

#endif /* VL53L1X_PORT_H */
