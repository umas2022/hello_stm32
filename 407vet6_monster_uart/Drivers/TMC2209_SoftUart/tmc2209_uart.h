#ifndef TMC2209_H
#define TMC2209_H

#include "stm32f4xx_hal.h"

void TMC2209_sendRawMessage(uint8_t instance, const uint8_t *message, size_t length);

// TMC2209初始化
void TMC2209_Init(uint8_t instance);

// TMC2209设置电机运行电流100%
void TMC2209_setRunCurrent_100(uint8_t instance);

// TMC2209启用coolstep
void TMC2209_enableCoolStep(uint8_t instance);

// TMC2209启动
void TMC2209_enable(uint8_t instance);

// TMC2209设置速度
void TMC2209_moveAtVelocity(uint8_t instance, uint32_t speed);

// TMC2209启用stallguard
void TMC2209_enableStallGuard(uint8_t instance);

#endif // TMC2209_H
