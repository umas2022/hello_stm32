#include <stdint.h>
#include <string.h>
#include <tmc2209_stm32.h>

#include "softuart.h"

// 速度脉冲间隔
#define SPEED_STEP_DIVISION 10

// 初始化多个电机
StepperMotorControl motors[] = {
/*Motor 1*/
{
//
		.targetSteps = 0, .currentSteps = 0, .speed = 0, .timsCounter = 0, .direction = 1, .isActive = 0,
		// pin:EN
		.port_en = GPIOC, .pin_en = GPIO_PIN_15,
		// pin:DIR
		.port_dir = GPIOC, .pin_dir = GPIO_PIN_13,
		// pin:STEP
		.port_step = GPIOC, .pin_step = GPIO_PIN_14,
		// pin:UART
		.port_uart = GPIOE, .pin_uart = GPIO_PIN_6, .uart_index = 0 }, };

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

			// TMC2209 GCONF 全局配置寄存器初始化
			TMC2209_setGconf(motors[i].uart_index);
			// 电流100%
			TMC2209_setRunCurrent_100(motors[i].uart_index);
			TMC2209_setRunCurrent(motors[i].uart_index, 50);
			// coolstep
			TMC2209_enableCoolStep(motors[i].uart_index);
			// stallguard
			TMC2209_enableStallGuard(motors[i].uart_index);
			// 2 microstep
			TMC2209_setMicrostepping(motors[i].uart_index, 1);
		}
	}
}

// 启动特定编号的电机
void TMC2209_startStepperMotor(uint8_t motorIndex, uint32_t steps, uint32_t speed, int8_t direction) {
	if (motorIndex < getNumMotors()) {
		StepperMotorControl *motor = &motors[motorIndex];
		motor->targetSteps = steps;
		motor->currentSteps = 0;
		motor->speed = (speed > 100) ? 100 : ((speed < 0) ? 0 : speed); // 约束 speed 在 [0, 100] 范围内
		motor->timsCounter = motor->targetSteps * 100 * SPEED_STEP_DIVISION / motor->speed;
		motor->direction = direction;
		motor->isActive = 1;

		// 设置使能信号
		HAL_GPIO_WritePin(motor->port_en, motor->pin_en, GPIO_PIN_RESET);

		// 设置初始方向
		HAL_GPIO_WritePin(motor->port_dir, motor->pin_dir, (direction > 0) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	}
}

// 定时器周期结束回调函数
void TMC2209_stepHandler(void) {
	for (uint8_t i = 0; i < getNumMotors(); ++i) {
		StepperMotorControl *motor = &motors[i];
		if (motor->speed == 0 || !motor->isActive) {
			continue;
		}

		// 检查是否应该发送脉冲
		if (motor->timsCounter % (100 * SPEED_STEP_DIVISION / motor->speed) == 0) {
			// 发送脉冲信号
			HAL_GPIO_TogglePin(motor->port_step, motor->pin_step);

			// 更新步数计数器
			motor->currentSteps += 1;

			// 检查是否达到目标步数
			if (motor->currentSteps >= motor->targetSteps) {
				motor->isActive = 0;
			}
		}
		motor->timsCounter -= 1;
	}
}

// 软件串口发送原始消息
void TMC2209_sendRawMessage(uint8_t instance, const uint8_t *message, size_t length) {
	uint8_t tempBuffer[length];
	memcpy(tempBuffer, message, length);
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

// TMC2209 GCONF 全局配置寄存器初始化
void TMC2209_setGconf(uint8_t instance) {
	const uint8_t msg[8] = { 0x05, 0x00, 0x80, 0x00, 0x00, 0x00, 0x83, 0x00 };
	TMC2209_sendRawMessage(instance, msg, sizeof(msg));
	HAL_Delay(10);
}

// TMC2209串口速度驱动
// 扭矩较小，暂不清楚原因，需要关闭stallguard
void TMC2209_moveAtVelocity(uint8_t instance, uint32_t speed) {
	// 停转信号
	uint8_t msg_init[8] ={ 0x05, 0x00, 0xA2, 0x00, 0x00, 0x00, 0x00, 0x0E };
	TMC2209_sendRawMessage(instance, msg_init, sizeof(msg_init));
	HAL_Delay(10);

	// 使能引脚
	HAL_GPIO_WritePin(motors[instance].port_en, motors[instance].pin_en, GPIO_PIN_RESET);

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

// TMC2209设置电机运行电流100%，保持电流50%
// ihold =16 = 50% , irun = 31 = 100%
// iholddelay = 10 = 2^10 timer period
void TMC2209_setRunCurrent_100(uint8_t instance) {
	const uint8_t msg_ihold_irun[] = { 0x05, 0x00, 0x90, 0x00, 0x01, 0x1F, 0x10, 0xDF };
	TMC2209_sendRawMessage(instance, msg_ihold_irun, sizeof(msg_ihold_irun));
	HAL_Delay(10);
}

// 设置电机运行电流IRUN
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
	const uint8_t msg_enable[] = { 0x05, 0x00, 0xC2, 0x00, 0x00, 0x80, 0x01, 0xD9 };
	TMC2209_sendRawMessage(instance, msg_enable, sizeof(msg_enable));
	HAL_Delay(10);
}

// TMC2209启用堵转检测功能和diag引脚输出
void TMC2209_enableStallGuard(uint8_t instance) {
	// 使能diag输出
	const uint8_t msg_tcoolthrs[8] = { 0x05, 0x00, 0x94, 0x00, 0x00, 0xFF, 0xFF, 0x10 };
	TMC2209_sendRawMessage(instance, msg_tcoolthrs, sizeof(msg_tcoolthrs));
	HAL_Delay(10);
	// 设置阈值
	uint8_t msg_sgthrs[8] = { 0x05, 0x00, 0xC0, 0x00, 0x00, 0x00, 0x40, 0x00 };
	msg_sgthrs[7] = calculate_crc(msg_sgthrs, sizeof(msg_sgthrs));
	TMC2209_sendRawMessage(instance, msg_sgthrs, sizeof(msg_sgthrs));
	HAL_Delay(10);
}

// 设置细分等级
void TMC2209_setMicrostepping(uint8_t instance, uint8_t mres_level) {
	// 约束 mres 在合法范围内 [0, 8]
	if (mres_level > 8) {
		mres_level = 8;
	}

	// 细分
	const uint8_t msg_microstep[][8] = {
	// 0-1
			{ 0x05, 0x00, 0xEC, 0x18, 0x01, 0x00, 0x53, 0xF0 },
			// 1-2
			{ 0x05, 0x00, 0xEC, 0x17, 0x01, 0x00, 0x53, 0xF3 },
			// 2-4
			{ 0x05, 0x00, 0xEC, 0x16, 0x01, 0x00, 0x53, 0xC2 },
			// 3-8
			{ 0x05, 0x00, 0xEC, 0x15, 0x01, 0x00, 0x53, 0x68 },
			// 4-16
			{ 0x05, 0x00, 0xEC, 0x14, 0x01, 0x00, 0x53, 0x59 },
			// 5-32
			{ 0x05, 0x00, 0xEC, 0x13, 0x01, 0x00, 0x53, 0x3D },
			// 6-64
			{ 0x05, 0x00, 0xEC, 0x12, 0x01, 0x00, 0x53, 0x0C },
			// 7-128
			{ 0x05, 0x00, 0xEC, 0x11, 0x01, 0x00, 0x53, 0xA6 },
			// 8-256
			{ 0x05, 0x00, 0xEC, 0x10, 0x01, 0x00, 0x53, 0x97 } };
	TMC2209_sendRawMessage(0, msg_microstep[mres_level], sizeof(msg_microstep[0]));
	HAL_Delay(10);
}
