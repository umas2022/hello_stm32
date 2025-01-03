#ifndef TMC2209_H
#define TMC2209_H

#include "stm32f1xx_hal.h"

void TMC2209_sendRawMessage(UART_HandleTypeDef *huart, const uint8_t *message,
		size_t length);

// TMC2209初始化
void TMC2209_Init(UART_HandleTypeDef *huart);

// TMC2209设置电机运行电流100%
void TMC2209_setRunCurrent_100(UART_HandleTypeDef *huart);

// TMC2209启用coolstep
void TMC2209_enableCoolStep(UART_HandleTypeDef *huart);

// TMC2209启动
void TMC2209_enable(UART_HandleTypeDef *huart);

// TMC2209设置速度
void TMC2209_moveAtVelocity(UART_HandleTypeDef *huart, uint32_t speed);

#endif // TMC2209_H
