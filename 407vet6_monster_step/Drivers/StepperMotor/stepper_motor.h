#ifndef STEPPER_MOTOR_H
#define STEPPER_MOTOR_H

#include "stm32f4xx_hal.h"
#include "main.h"

// 定义电机控制结构体
typedef struct {
    uint32_t targetSteps;     // 目标步数
    uint32_t currentSteps;    // 当前已完成步数
    float speed;              // 转速 (单位时间内的步数)
    int8_t direction;         // 方向：1为正转，-1为反转
    uint8_t isActive;         // 是否正在运行
    GPIO_TypeDef* port_en;    // 使能端口
    uint16_t pin_en;          // 使能引脚
    GPIO_TypeDef* port_dir;   // 方向端口
    uint16_t pin_dir;         // 方向引脚
    GPIO_TypeDef* port_step;  // 脉冲端口
    uint16_t pin_step;        // 脉冲引脚
} StepperMotorControl;

// 外部变量声明
extern StepperMotorControl motors[];

// 函数原型声明
void startStepperMotor(uint8_t motorIndex, uint32_t steps, float speed, int8_t direction);
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);

#endif // STEPPER_MOTOR_H
