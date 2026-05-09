#ifndef __TFMINIPLUS_H__
#define __TFMINIPLUS_H__

/**
 * @file tfminiplus.h
 * @brief TFminiPlus 激光雷达测距模块（I2C抽象）
 */

#include <stdint.h>

/**
 * @brief I2C发送函数指针
 * @param dev_addr  设备地址
 * @param data      发送数据
 * @param len       数据长度
 * @return 0=成功
 */
typedef int (*TFMini_I2CSend_t)(uint8_t dev_addr, const uint8_t *data, uint16_t len);

/**
 * @brief I2C接收函数指针
 * @param dev_addr  设备地址
 * @param data      接收缓冲区
 * @param len       接收长度
 * @return 0=成功
 */
typedef int (*TFMini_I2CRecv_t)(uint8_t dev_addr, uint8_t *data, uint16_t len);

typedef struct
{
    uint8_t dev_addr;
    uint8_t Mode;
    uint16_t Dist;
    uint32_t Strength;
    uint8_t buf[11];
    TFMini_I2CSend_t i2c_send;
    TFMini_I2CRecv_t i2c_recv;
} TFMiniInstance;

/**
 * @brief 初始化TFminiPlus
 * @param inst       实例指针(由调用者分配)
 * @param dev_addr   I2C设备地址(默认0x10)
 * @param send_fn    I2C发送函数
 * @param recv_fn    I2C接收函数
 */
void TFMiniInit(TFMiniInstance *inst, uint8_t dev_addr,
                TFMini_I2CSend_t send_fn, TFMini_I2CRecv_t recv_fn);

/**
 * @brief 获取测距结果
 * @param inst TFmini实例
 * @return 距离值(mm)
 */
float TFMiniGetDistance(TFMiniInstance *inst);

#endif
