#include "step_motor.h"
#include "X_V2.h"
#include <string.h>

#define STEP_MOTOR_MAX_ADDR 8
#define STEP_MOTOR_UART_GROUPS 2

static StepMotor_UartSend_t uart_send_fn[STEP_MOTOR_UART_GROUPS] = {0};
static uint8_t addr_to_group[STEP_MOTOR_MAX_ADDR + 1] = {0}; // 默认全部映射到group 0

void StepMotor_RegisterUart(uint8_t group, StepMotor_UartSend_t send_fn)
{
    if (group < STEP_MOTOR_UART_GROUPS)
    {
        uart_send_fn[group] = send_fn;
    }
}

void StepMotor_SetAddrGroup(uint8_t addr, uint8_t group)
{
    if (addr <= STEP_MOTOR_MAX_ADDR && group < STEP_MOTOR_UART_GROUPS)
    {
        addr_to_group[addr] = group;
    }
}

void step_sendcmd(uint8_t addr, uint8_t *cmd, uint16_t len)
{
    if (len > 0 && cmd != NULL)
    {
        uint8_t group = 0;
        if (addr <= STEP_MOTOR_MAX_ADDR)
            group = addr_to_group[addr];

        if (uart_send_fn[group] != NULL)
        {
            uart_send_fn[group](cmd, len);
        }
    }
}
