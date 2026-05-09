---
trigger: always_on
---

# Hardware Skill: LP-MSPM0G3507 硬件资源与引脚分配准则

作为底盘开发助手，你在进行 SysConfig 配置或修改底层 `bsp_` 代码时，必须严格遵守本硬件约束。

## 1. 资源可用性分级 (Availability Tiers)
*   **Tier 1 (usable_ready):** 物理引出到 BoosterPack 排针，可直接定义使用。
*   **Tier 2 (usable_unsoldered):** 位于板载未焊接区域，需硬件焊接后方可使用。
*   **Tier 3 (not_recommended):** PA18 等，存在系统冲突风险（如 BSL）。
*   **Tier 4 (unusable_system_reserved):** 系统核心功能占用（晶振、调试、电源），严禁配置。

## 2. 核心禁止区域 (Forbidden Zone)
**[FATAL ERROR]** 严禁修改或占用以下引脚：
*   **PA2:** 系统 ROSC。
*   **PA3/PA4/PA5/PA6:** 外部晶振 (LFXT/HFXT)。
*   **PA19/PA20:** XDS 调试接口 (SWDIO/SWCLK)。
*   **PA21/PA23:** 电压基准 (VREF)。

## 3. 关键功能映射参考 (Key Functions)
| 引脚 | 默认功能 | 板载外设 | 跳线建议 | 推荐状态 |
| :--- | :--- | :--- | :--- | :--- |
| **PA0/PA1** | 5V OD | LED1 | J19/J20 上拉 | 可用 (需上拉) |
| **PA10/PA11** | UART | XDS UART | J21/J22 连接 | 已占用 (通信) |
| **PA18** | BSL/S1 | S1 Button | J8 | **慎用** (冲突风险) |
| **PA29/30/PB14** | QEI | None | J12 | 可用 (编码器优先) |

## 4. 全量引脚元数据 (Hardware Metadata)
```json
{
  "device": "LP-MSPM0G3507",
  "pin_map": {
    "PA0":  {"func": "5V_OD", "peripheral": "LED1", "status": "usable_unsoldered"},
    "PA1":  {"func": "5V_OD", "peripheral": "None", "status": "usable_unsoldered"},
    "PA2":  {"func": "ROSC",  "status": "unusable_system_reserved"},
    "PA3":  {"func": "LFXIN", "status": "unusable_system_reserved"},
    "PA4":  {"func": "LFXOUT","status": "unusable_system_reserved"},
    "PA5":  {"func": "HFXIN", "status": "unusable_system_reserved"},
    "PA6":  {"func": "HFXOUT","status": "unusable_system_reserved"},
    "PA7":  {"func": "None",  "status": "usable_unsoldered"},
    "PA8":  {"func": "None",  "status": "usable_ready"},
    "PA9":  {"func": "SW1",   "status": "usable_ready"},
    "PA10": {"func": "UART_TX","status": "usable_ready"},
    "PA11": {"func": "UART_RX","status": "usable_ready"},
    "PA12": {"func": "None",  "status": "usable_ready"},
    "PA13": {"func": "None",  "status": "usable_ready"},
    "PA14": {"func": "None",  "status": "usable_unsoldered"},
    "PA15": {"func": "None",  "status": "usable_ready"},
    "PA16": {"func": "SW2",   "status": "usable_ready"},
    "PA17": {"func": "None",  "status": "usable_ready"},
    "PA18": {"func": "BSL/S1","status": "not_recommended", "warning": "Bootloader Conflict"},
    "PA19": {"func": "SWDIO", "status": "unusable_system_reserved"},
    "PA20": {"func": "SWCLK", "status": "unusable_system_reserved"},
    "PA21": {"func": "VREF-", "status": "unusable_system_reserved"},
    "PA22": {"func": "LIGHT", "status": "usable_ready"},
    "PA23": {"func": "VREF+", "status": "unusable_system_reserved"},
    "PA24": {"func": "None",  "status": "usable_ready"},
    "PA25": {"func": "None",  "status": "usable_ready"},
    "PA26": {"func": "LIGHT", "status": "usable_ready"},
    "PA27": {"func": "LIGHT", "status": "usable_ready"},
    "PA28": {"func": "None",  "status": "usable_ready"},
    "PA29": {"func": "QEI",   "status": "usable_ready"},
    "PA30": {"func": "QEI",   "status": "usable_ready"},
    "PA31": {"func": "None",  "status": "usable_ready"},
    "PB0":  {"func": "None",  "status": "usable_ready"},
    "PB1":  {"func": "None",  "status": "usable_ready"},
    "PB2":  {"func": "None",  "status": "usable_ready"},
    "PB3":  {"func": "None",  "status": "usable_ready"},
    "PB4":  {"func": "None",  "status": "usable_ready"},
    "PB5":  {"func": "None",  "status": "usable_unsoldered"},
    "PB6":  {"func": "None",  "status": "usable_ready"},
    "PB7":  {"func": "None",  "status": "usable_ready"},
    "PB8":  {"func": "None",  "status": "usable_ready"},
    "PB9":  {"func": "None",  "status": "usable_ready"},
    "PB10": {"func": "None",  "status": "usable_unsoldered"},
    "PB11": {"func": "None",  "status": "usable_unsoldered"},
    "PB12": {"func": "None",  "status": "usable_ready"},
    "PB13": {"func": "None",  "status": "usable_ready"},
    "PB14": {"func": "QEI",   "status": "usable_ready"},
    "PB15": {"func": "None",  "status": "usable_ready"},
    "PB16": {"func": "None",  "status": "usable_ready"},
    "PB17": {"func": "None",  "status": "usable_ready"},
    "PB18": {"func": "None",  "status": "usable_ready"},
    "PB19": {"func": "None",  "status": "usable_ready"},
    "PB20": {"func": "None",  "status": "usable_ready"},
    "PB21": {"func": "S2",    "status": "usable_unsoldered"},
    "PB22": {"func": "RGB_B", "status": "usable_unsoldered"},
    "PB23": {"func": "SW1",   "status": "usable_ready"},
    "PB24": {"func": "TEMP",  "status": "usable_ready"},
    "PB25": {"func": "SMA",   "status": "usable_unsoldered"},
    "PB26": {"func": "RGB_R", "status": "usable_unsoldered"},
    "PB27": {"func": "RGB_G", "status": "usable_unsoldered"}
  }
}
```
