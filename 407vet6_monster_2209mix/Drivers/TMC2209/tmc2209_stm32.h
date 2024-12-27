#ifndef TMC2209_STM32_H
#define TMC2209_STM32_H

#include "stm32f4xx_hal.h"

// 定义电机控制结构体
typedef struct {
	uint32_t targetSteps;     // 目标步数
	uint32_t currentSteps;    // 当前已完成步数
	uint32_t speed;              // 转速 (0-100)
	uint32_t timsCounter;	// 中断总数
	int8_t direction;         // 方向：1为正转，-1为反转
	uint8_t isActive;         // 是否正在运行
	GPIO_TypeDef *port_en;    // 使能端口
	uint16_t pin_en;          // 使能引脚
	GPIO_TypeDef *port_dir;   // 方向端口
	uint16_t pin_dir;         // 方向引脚
	GPIO_TypeDef *port_step;  // 脉冲端口
	uint16_t pin_step;        // 脉冲引脚
	GPIO_TypeDef *port_uart;  // 串口端口
	uint16_t pin_uart;        // 串口引脚
	uint16_t uart_index;  // 串口编号
} StepperMotorControl;

// 外部变量声明
extern StepperMotorControl motors[];

// 初始化电机组
void motors_setup(void);

// 电机运动控制
void TMC2209_startStepperMotor(uint8_t motorIndex, uint32_t steps, uint32_t speed, int8_t direction);

// 电机运动控制定时器中断回调
void TMC2209_stepHandler(void);

// 软件串口发送消息
void TMC2209_sendRawMessage(uint8_t instance, const uint8_t *message, size_t length);

// TMC2209 GCONF 全局配置寄存器初始化
void TMC2209_setGconf(uint8_t instance);

// TMC2209设置电机运行电流100%
void TMC2209_setRunCurrent_100(uint8_t instance);
// TMC2209设置电机运行电流
void TMC2209_setRunCurrent(uint8_t instance, uint8_t percentage);

// TMC2209启用coolstep
void TMC2209_enableCoolStep(uint8_t instance);

// TMC2209设置速度
void TMC2209_moveAtVelocity(uint8_t instance, uint32_t speed);

// TMC2209启用stallguard
void TMC2209_enableStallGuard(uint8_t instance);

// TMC2209设置细分等级
void TMC2209_setMicrostepping(uint8_t instance, uint8_t mres_value);

#endif // TMC2209_STM32_H
