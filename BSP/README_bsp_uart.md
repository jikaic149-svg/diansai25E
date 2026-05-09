# 串口BSP模块使用文档

## 概述
本 BSP 提供针对 MSPM0G350X 的两个 UART 通道的外设封装。它们均处于 DMA 与空闲（RX_TIMEOUT_ERROR）中断双轮驱动的工作流下，可保证以数据包（Packet）为单位处理数据，无需逐字节干预 CPU。
- `UART1` (UART_WIT)：JY61P传感器（仅接收，DMA）
- `UART2` (UART_Blue)：蓝牙透传模块（发送接收均用 DMA）

## 依赖
- 需要预先在 `empty.syscfg` 对应的 SysConfig 图形界面中为各个 UART 生成对应参数与中断使能。
- 已配置好相应的 `ti_msp_dl_config.h` 相关引脚定义与系统时钟。

## 使用方法

### 1. 初始化
在 `main()` 函数中，一旦调用完 `SYSCFG_DL_init();`，需调用串口初始化：
```c
#include "bsp_uart.h"

int main(void) {
    SYSCFG_DL_init();
    BSP_UART_Init();  // 开启串口 DMA 通道与 NVIC 中断
    
    while(1) {
        // ...
    }
}
```

### 2. 接收 JY61P 或 蓝牙 数据包
每当串口总线空闲（一帧数据接收完毕），框架会自动重置 DMA 并在后台调用对应的弱函数（Weak Function）回调。在您的 `main.c` 或应用层 `app.c` 中需按如下形式予以重写：
```c
void BSP_WIT_ProcessPacket(uint8_t *data, uint16_t len) {
    // 示例：解析 JY61P 帧头 0x55 并打印或其他处理
}

void BSP_BT_ProcessPacket(uint8_t *data, uint16_t len) {
    // 示例：校验蓝牙透传接收到的控制指令
}
```

### 3. 发送 蓝牙 数据包
随时随地在代码的逻辑块或中断内，使用 `BSP_BT_SendPacket()` 非阻塞式地发送数据。
```c
uint8_t test_data[] = {0x01, 0x02, 0x03, 0x04};
BSP_BT_SendPacket(test_data, sizeof(test_data));
```
**注意**：`test_data` 应当保证在 DMA 传输期间不被释放（不可是已销毁的栈顶局部变量）。如果需要在循环内不断发送，可考虑添加传输完成后的阻塞判断（检查 DMA Channel 激活状态）或者做队列处理。
