#include <stdint.h>
#include <string.h>
#include <tmc2209_stm32.h>

#include "softuart.h"


// 初始化多个电机
StepperMotorControl motors[] = {
	// Motor 1
    [0] = {
        .targetSteps = 0,
        .currentSteps = 0,
        .speed = 0,
        .direction = 1,
        .isActive = 0,
        .port_en = GPIOC,
        .pin_en = GPIO_PIN_15,
        .port_dir = GPIOC,
        .pin_dir = GPIO_PIN_13,
        .port_step = GPIOC,
        .pin_step = GPIO_PIN_14,
        .port_uart = GPIOE,
        .pin_uart = GPIO_PIN_6,
        .uart_index = 0
    },
    // 可以继续添加更多电机...
};


// 获取电机数量
static inline uint8_t getNumMotors(void) {
    return sizeof(motors) / sizeof(motors[0]);
}

// 初始化电机组
void motors_setup(void) {
    for (size_t i = 0; i < getNumMotors(); ++i) {
        // 检查是否有有效的 UART 端口配置
        if (motors[i].port_uart != NULL && motors[i].pin_uart != 0) {
            // 初始化电机的 UART 端口
            SoftUartInit(motors[i].uart_index, motors[i].port_uart, motors[i].pin_uart, NULL, 0);
            // 串口参数初始化
        	TMC2209_Init(motors[i].uart_index);
        	// 电流100%
        	TMC2209_setRunCurrent_100(motors[i].uart_index);
//        	TMC2209_setRunCurrent(motors[i].uart_index,50);
        	// 使能coolstep
        	TMC2209_enableCoolStep(motors[i].uart_index);
        	// 使能电机
        	TMC2209_enable(motors[i].uart_index);
        	// 使能堵转检测
        	TMC2209_enableStallGuard(motors[i].uart_index);
        }
    }
}


// 启动特定编号的电机
void TMC2209_startStepperMotor(uint8_t motorIndex, uint32_t steps, float speed, int8_t direction) {
    if (motorIndex < getNumMotors()) {
        StepperMotorControl *motor = &motors[motorIndex];
        motor->targetSteps = steps;
        motor->currentSteps = 0;
        motor->speed = speed;
        motor->direction = direction;
        motor->isActive = 1;

        // 设置使能信号
        HAL_GPIO_WritePin(motor->port_en, motor->pin_en, GPIO_PIN_RESET);

        // 设置初始方向
        HAL_GPIO_WritePin(motor->port_dir, motor->pin_dir, (direction > 0) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }
}



// 定时器周期结束回调函数
void TMC2209_stepHandler(void){
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
}

// 软件串口发送原始消息
void TMC2209_sendRawMessage(uint8_t instance, const uint8_t *message, size_t length) {
    // 创建一个非const的中间缓冲区
    uint8_t tempBuffer[length];
    memcpy(tempBuffer, message, length);

    // 使用中间缓冲区调用 SoftUartPuts
    SoftUartPuts(instance, tempBuffer, length);
}

// CRC8 calculation function
uint8_t calculate_crc(unsigned char *datagram, size_t length) {
	unsigned char crc = 0;

	// Iterate over each byte except the last one (assumed to be for CRC)
	for (size_t i = 0; i < length - 1; i++) {
		unsigned char byte = datagram[i];

		// Process each bit of the byte
		for (int j = 0; j < 8; j++) {
			if (((crc >> 7) ^ (byte & 0x01)) != 0) {
				crc = ((crc << 1) ^ 0x07) & 0xFF;
			} else {
				crc = (crc << 1) & 0xFF;
			}

			// Shift byte to process next bit
			byte = byte >> 1;
		}
	}

	return crc;
}

// TMC2209初始化，512细分
void TMC2209_Init(uint8_t instance) {
	const uint8_t msg_setup[][8] = { { 0x05, 0x00, 0x80, 0x00, 0x00, 0x01, 0xC0,
			0xF6 }, { 0x05, 0x00, 0x90, 0x00, 0x01, 0x1F, 0x10, 0xDF }, { 0x05,
			0x00, 0xEC, 0x10, 0x01, 0x00, 0x53, 0x97 }, { 0x05, 0x00, 0xF0,
			0xC1, 0x0D, 0x00, 0x24, 0x06 }, { 0x05, 0x00, 0xC2, 0x00, 0x00,
			0x00, 0x00, 0x45 },
			{ 0x05, 0x00, 0x91, 0x00, 0x00, 0x00, 0x14, 0x1F }, { 0x05, 0x00,
					0x93, 0x00, 0x00, 0x00, 0x00, 0x0F }, { 0x05, 0x00, 0xA2,
					0x00, 0x00, 0x00, 0x00, 0x0E }, { 0x05, 0x00, 0x94, 0x00,
					0x00, 0x00, 0x00, 0x34 }, { 0x05, 0x00, 0xC0, 0x00, 0x00,
					0x00, 0x00, 0x8D }, { 0x05, 0x00, 0xC2, 0x00, 0x00, 0x00,
					0x00, 0x45 }, { 0x05, 0x00, 0x81, 0x00, 0x00, 0x00, 0x02,
					0x19 }, { 0x05, 0x00, 0x90, 0x00, 0x01, 0x00, 0x00, 0x5B },
			{ 0x05, 0x00, 0xEC, 0x10, 0x01, 0x00, 0x50, 0xD9 }, { 0x05, 0x00,
					0xF0, 0xC1, 0x09, 0x00, 0x24, 0x45 }, { 0x05, 0x00, 0xF0,
					0xC1, 0x01, 0x00, 0x24, 0xE7 } };
    // 发送设置消息（逐条发送）
    for (size_t i = 0; i < sizeof(msg_setup) / sizeof(msg_setup[0]); ++i) {
        TMC2209_sendRawMessage(instance, msg_setup[i], sizeof(msg_setup[i]));
        HAL_Delay(10); // 短暂延迟以确保每条消息都被正确接收
    }
}



// TMC2209启动
void TMC2209_enable(uint8_t instance) {
	const uint8_t msg_enable[] = { 0x05, 0x00, 0xEC, 0x10, 0x01, 0x00, 0x53,
			0x97 };
	TMC2209_sendRawMessage(instance, msg_enable, sizeof(msg_enable));
	HAL_Delay(10);
}


// TMC2209设置速度
void TMC2209_moveAtVelocity(uint8_t instance, uint32_t speed) {
	uint8_t msg_move[8] = { 0x05, 0x00, 0xA2, 0x00, 0x00, 0x00, 0x00, 0x00 };

	// 设置速度值（大端模式）
	msg_move[3] = (speed >> 24) & 0xFF;  // 速度的高字节
	msg_move[4] = (speed >> 16) & 0xFF;  // 速度的次高字节
	msg_move[5] = (speed >> 8) & 0xFF;   // 速度的次低字节
	msg_move[6] = speed & 0xFF;          // 速度的低字节
	uint8_t crc = calculate_crc(msg_move, sizeof(msg_move));
	msg_move[7] = crc;  // 将CRC值存入消息的最后一个字节

	// 发送消息
	TMC2209_sendRawMessage(instance, msg_move, sizeof(msg_move));
	HAL_Delay(10);
}


// TMC2209设置电机运行电流100%
void TMC2209_setRunCurrent_100(uint8_t instance) {
	const uint8_t msg_current[] = { 0x05, 0x00, 0x90, 0x00, 0x01, 0x1F, 0x00,
			0xE7 };
	TMC2209_sendRawMessage(instance, msg_current, sizeof(msg_current));
	HAL_Delay(10);
}


// 设置电机运行电流
void TMC2209_setRunCurrent(uint8_t instance, uint8_t percent) {
    // 定义最大和最小的百分比以及对应的寄存器设置值
    const uint8_t percent_min = 0;
    const uint8_t percent_max = 100;
    const uint8_t current_setting_min = 0;
    const uint8_t current_setting_max = 31; // 对应于IRUN的最大值

    // 约束输入值在[percent_min, percent_max]范围内
    if (percent > percent_max) {
        percent = percent_max;
    } else if (percent < percent_min) {
        percent = percent_min;
    }

    // 将百分比映射到电流设置值
    uint8_t run_current = ((percent * (current_setting_max - current_setting_min)) / percent_max) + current_setting_min;

    // 构建消息
    uint8_t msg_current[8] = { 0x05, 0x00, 0x90, 0x00, 0x01, 0x1F, 0x00, 0x00 };

    // 更新 IRUN 字段（最低5位）
    msg_current[5] = (msg_current[5] & 0xE0) | (run_current & 0x1F);

    // 更新 CRC 校验码
    uint8_t crc = calculate_crc(msg_current, sizeof(msg_current));
    msg_current[7] = crc;

    // 发送消息
    TMC2209_sendRawMessage(instance, msg_current, sizeof(msg_current));
    HAL_Delay(10); // 等待一段时间确保消息被正确处理
}


// TMC2209启用coolstep
void TMC2209_enableCoolStep(uint8_t instance) {
	const uint8_t msg_enable[] = { 0x05, 0x00, 0xEC, 0x10, 0x01, 0x00, 0x53,
			0x97 };
	TMC2209_sendRawMessage(instance, msg_enable, sizeof(msg_enable));
	HAL_Delay(10);
}

// TMC2209使能堵转检测功能和diag引脚输出
void TMC2209_enableStallGuard(uint8_t instance) {
	// 使能diag输出
	const uint8_t msg_tcoolthrs[] = { 0x05, 0x00, 0x94, 0x00, 0x00, 0xFF, 0xFF, 0x10 };
	TMC2209_sendRawMessage(instance, msg_tcoolthrs, sizeof(msg_tcoolthrs));
	HAL_Delay(10);
	// 设置阈值
	const uint8_t msg_sgthrs[] = { 0x05, 0x00, 0xC0, 0x00, 0x00, 0x10, 0x00, 0x83 };
	TMC2209_sendRawMessage(instance, msg_sgthrs, sizeof(msg_sgthrs));
	HAL_Delay(10);
}

