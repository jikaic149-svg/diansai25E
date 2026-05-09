/**
 * @file    icm42688.h
 * @brief   OOP架构六轴 IMU 底层解耦驱动 (彻底剥离 TI 及特定单片机宏)
 */
#ifndef ICM42688_H
#define ICM42688_H

#include <stdint.h>

/** 
 * @brief  脱离死代码：这 3 个抽象指针是替换原本的硬件交互命脉 
 */
typedef uint8_t (*ICM_SPI_ReadWrite_Func_t)(uint8_t tx_data);
typedef void    (*ICM_CS_Control_Func_t)(uint8_t level);  // 0:选通, 1:释放
typedef void    (*ICM_DelayMs_Func_t)(uint32_t ms);

/**
 * @brief IMU 对象的实例容器，可以声明为数组操控多个独立的六轴
 */
typedef struct {
    ICM_SPI_ReadWrite_Func_t SPI_ReadWrite;
    ICM_CS_Control_Func_t    CS_Control;
    ICM_DelayMs_Func_t       Delay_Ms;
    
    // 动态量程和灵敏度记录
    float accSensitivity;
    float gyroSensitivity;
} ICM42688_t;

/**
 * @brief 标准化解析后的实测三轴输出
 */
typedef struct {
    float x; 
    float y; 
    float z;
} ICM42688_Data_t;


/* ======= OOP 式的实例操控与原厂 API ======= */
int8_t ICM42688_Init(ICM42688_t *inst);
int8_t ICM42688_ReadRegs(ICM42688_t *inst, uint8_t reg, uint8_t* buf, uint16_t len);
int8_t ICM42688_WriteReg(ICM42688_t *inst, uint8_t reg, uint8_t value);
int8_t ICM42688_GetRawData(ICM42688_t *inst, ICM42688_Data_t *acc, ICM42688_Data_t *gyro);

#endif /* ICM42688_H */
