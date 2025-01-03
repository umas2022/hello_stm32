#include "stepper_motor.h"
#include "main.h" // 确保包含了必要的硬件初始化（如定时器配置）


// 初始化多个电机
StepperMotorControl motors[] = {
    {0, 0, 0, 1, 0,GPIOC, GPIO_PIN_15, GPIOC, GPIO_PIN_13,GPIOC, GPIO_PIN_14}, // Motor 1
};

// 获取电机数量
static inline uint8_t getNumMotors(void) {
    return sizeof(motors) / sizeof(motors[0]);
}

// 启动特定编号的电机
void startStepperMotor(uint8_t motorIndex, uint32_t steps, float speed, int8_t direction) {
    if (motorIndex < getNumMotors()) {
        StepperMotorControl *motor = &motors[motorIndex];
        motor->targetSteps = steps;
        motor->currentSteps = 0;
        motor->speed = speed;
        motor->direction = direction;
        motor->isActive = 1;

//        // 如果还没有启动定时器，则启动它
//        static uint8_t timerStarted = 0;
//        if (!timerStarted) {
//            HAL_TIM_Base_Start_IT(&htim5);
//            timerStarted = 1;
//        }

        // 设置使能信号
        HAL_GPIO_WritePin(motor->port_en, motor->pin_en, GPIO_PIN_RESET);

        // 设置初始方向
        HAL_GPIO_WritePin(motor->port_dir, motor->pin_dir, (direction > 0) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }
}



// 定时器周期结束回调函数
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM5) {
        for (uint8_t i = 0; i < getNumMotors(); ++i) {
            StepperMotorControl *motor = &motors[i];
            if (motor->isActive) {
                // 发送脉冲信号
                HAL_GPIO_TogglePin(motor->port_step, motor->pin_step);

                // 更新步数计数器
                motor->currentSteps += motor->direction;

                // 检查是否达到目标步数
                if ((motor->direction > 0 && motor->currentSteps >= motor->targetSteps) ||
                    (motor->direction < 0 && motor->currentSteps <= 0)) {
                    motor->isActive = 0;
                    // 关闭使能信号
                    HAL_GPIO_WritePin(motor->port_en, motor->pin_en, GPIO_PIN_RESET);
                    // 可选: 执行完成后的回调或通知主程序
                }
            }
        }

//        // 如果没有电机正在运行，可以选择停止定时器
//        uint8_t anyActive = 0;
//        for (uint8_t i = 0; i < getNumMotors(); ++i) {
//            if (motors[i].isActive) {
//                anyActive = 1;
//                break;
//            }
//        }
//        if (!anyActive) {
//            HAL_TIM_Base_Stop_IT(&htim5);
//        }
    }
}
