#include <stdint.h>
#include <string.h>
#include <tmc2209_stm32.h>

#include "softuart.h"

// 设置寄存器GCONF
RegisterGlobalConfig register_gconf;
void setRegisterGCONF(uint8_t uart_instance) {
	// 设置特定位
	register_gconf.i_scale_analog = 1;           // 使用外部VREF
	register_gconf.internal_rsense = 0;          // 使用外部感应电阻
	register_gconf.enable_spread_cycle = 0;      // StealthChop 模式
	register_gconf.shaft = 0;                    // 正常电机方向
	register_gconf.index_otpw = 0;               // INDEX 显示序列发生器的第一个微步位置
	register_gconf.index_step = 0;               // 根据 index_otpw 的选择
	register_gconf.pdn_disable = 1;              // 禁用 PDN_UART 输入功能
	register_gconf.mstep_reg_select = 1;         // 使用 MRES 寄存器选择微步分辨率
	register_gconf.multistep_filt = 1;           // 启用步进脉冲过滤
	register_gconf.test_mode = 0;                // 正常操作模式
	write_register(uart_instance, ADDRESS_GCONF, register_gconf.bytes);
}

// 设置寄存器IHOLD_IRUN
// irun>=16时堵转检测数值不稳定
RegisterCurrent register_iholdirun;
void setRegisterIHOLDIRUN(uint8_t uart_instance) {
	register_iholdirun.bytes = 0;
	// 取值0-31
	register_iholdirun.ihold = 16;
	register_iholdirun.irun = 15;
	write_register(uart_instance, ADDRESS_IHOLD_IRUN, register_iholdirun.bytes);

}

// 设置寄存器COOLCONF
// 在速度semin以下启用低电流，semax以上禁用低电流
// semin = 1，semax = 0：低速时（semin = 1）启用低电流，其他任何速度禁用低电流
RegisterCoolConfig register_coolconf;
void setRegisterCOOLCONF(uint8_t uart_instance) {
	register_coolconf.bytes=0;
	// 取值0-15
	register_coolconf.semin = 1; // lower_threshold: min = 1, max = 15
	register_coolconf.semax = 0;  // upper_threshold: min = 0, max = 15, 0-2 recommended
	write_register(uart_instance, ADDRESS_COOLCONF, register_coolconf.bytes);
}

// 设置寄存器CHOPCONF
RegisterChopperConfig register_chopconf;
void setRegisterCHOPCONF(uint8_t uart_instance) {
	register_chopconf.bytes = 0x10000053;
	register_chopconf.tbl = 0b10;
	register_chopconf.hend = 0;
	register_chopconf.hstart = 5;
	register_chopconf.toff = 3;
	// 微步细分
	register_chopconf.mres = MRES_002;
	write_register(uart_instance, ADDRESS_CHOPCONF, register_chopconf.bytes);
}

// 寄存器TCOOLTHRS
// CoolStep 和 StallGuard 功能的下限速度阈值，当电机的速度超过这个阈值时，CoolStep 和 StallGuard 功能将被启用
RegisterTCOOLTHRS register_tcoolthrs;
void setRegisterTCOOLTHRS(uint8_t uart_instance){
	register_tcoolthrs.bytes = 0;
	register_tcoolthrs.threshold = 0xFFFF; // 设置下限速度阈值
//	register_tcoolthrs.threshold = 0xFFFFF; // 设置下限速度阈值
    write_register(uart_instance, ADDRESS_TCOOLTHRS, register_tcoolthrs.bytes);
}

// 寄存器SGTHRS
// threshold值越小，堵转力矩越大
// threshold值与register_iholdirun.irun关联，irun>=16时堵转检测数值不稳定
RegisterSGTHRS register_sgthrs;
void setRegisterSGTHRS(uint8_t uart_instance){
	register_sgthrs.bytes = 0;
	// threshold: 0-255
//	register_sgthrs.threshold = 128; // 设置检测阈值
	register_sgthrs.threshold = 64; // 设置检测阈值
    write_register(uart_instance, ADDRESS_SGTHRS, register_sgthrs.bytes);
}

// 写寄存器
void write_register(uint8_t uart_instance, uint8_t register_address, uint32_t data) {

	WriteReadReplyDatagram write_datagram;
	write_datagram.bytes = 0;
	write_datagram.sync = SYNC;
	write_datagram.serial_address = 0;
	write_datagram.register_address = register_address;
	write_datagram.rw = RW_WRITE;
	write_datagram.data = reverseData(data);

	uint8_t buffer_crc[WRITE_READ_REPLY_DATAGRAM_SIZE];
	memcpy(buffer_crc, &write_datagram.bytes, WRITE_READ_REPLY_DATAGRAM_SIZE);
	write_datagram.crc = calculate_crc(buffer_crc, WRITE_READ_REPLY_DATAGRAM_SIZE);

	uint8_t buffer[WRITE_READ_REPLY_DATAGRAM_SIZE];

	for (uint8_t i = 0; i < WRITE_READ_REPLY_DATAGRAM_SIZE; ++i) {
		buffer[i] = (write_datagram.bytes >> (i * BITS_PER_BYTE)) & BYTE_MAX_VALUE;
	}

	TMC2209_sendRawMessage(uart_instance, buffer, WRITE_READ_REPLY_DATAGRAM_SIZE);
	HAL_Delay(10);

}

// 反转32位数据
uint32_t reverseData(uint32_t data) {
	uint32_t reversed_data = 0;
	for (uint8_t i = 0; i < DATA_SIZE; ++i) {
		uint8_t right_shift = (DATA_SIZE - i - 1) * BITS_PER_BYTE;
		uint8_t left_shift = i * BITS_PER_BYTE;
		reversed_data |= ((data >> right_shift) & BYTE_MAX_VALUE) << left_shift;
	}
	return reversed_data;
}

// CRC8 calculation function
uint8_t calculate_crc(unsigned char *datagram, size_t length) {
	unsigned char crc = 0;

	for (size_t i = 0; i < length - 1; i++) {
		unsigned char byte = datagram[i];

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

// 初始化多个电机
StepperMotorControl motors[];
void motors_define() {
	// Motor 0
	motors[0].targetSteps = 0;
	motors[0].currentSteps = 0;
	motors[0].speed = 0;
	motors[0].timsCounter = 0;
	motors[0].direction = 1;
	motors[0].isActive = 0;

	motors[0].port_en = GPIOC;
	motors[0].pin_en = GPIO_PIN_15;
	motors[0].port_dir = GPIOC;
	motors[0].pin_dir = GPIO_PIN_13;
	motors[0].port_step = GPIOC;
	motors[0].pin_step = GPIO_PIN_14;
	motors[0].port_uart = GPIOE;
	motors[0].pin_uart = GPIO_PIN_6;
	motors[0].uart_index = 0;

	// Motor 1
	motors[1].targetSteps = 0;
	motors[1].currentSteps = 0;
	motors[1].speed = 0;
	motors[1].timsCounter = 0;
	motors[1].direction = 1;
	motors[1].isActive = 0;

//	motors[1].port_en = GPIOC;
//	motors[1].pin_en = GPIO_PIN_15;
//	motors[1].port_dir = GPIOE;
//	motors[1].pin_dir = GPIO_PIN_4;
//	motors[1].port_step = GPIOE;
//	motors[1].pin_step = GPIO_PIN_5;
//	motors[1].port_uart = GPIOE;
//	motors[1].pin_uart = GPIO_PIN_3;
//	motors[1].uart_index = 0;
//
//	// Motor 2
//	motors[2].targetSteps = 0;
//	motors[2].currentSteps = 0;
//	motors[2].speed = 0;
//	motors[2].timsCounter = 0;
//	motors[2].direction = 1;
//	motors[2].isActive = 0;
//
//	motors[2].port_en = GPIOE;
//	motors[2].pin_en = GPIO_PIN_2;
//	motors[2].port_dir = GPIOE;
//	motors[2].pin_dir = GPIO_PIN_0;
//	motors[2].port_step = GPIOE;
//	motors[2].pin_step = GPIO_PIN_1;
//	motors[2].port_uart = GPIOB;
//	motors[2].pin_uart = GPIO_PIN_7;
//	motors[2].uart_index = 0;
//
//	// Motor 3
//	motors[3].targetSteps = 0;
//	motors[3].currentSteps = 0;
//	motors[3].speed = 0;
//	motors[3].timsCounter = 0;
//	motors[3].direction = 1;
//	motors[3].isActive = 0;
//
//	motors[3].port_en = GPIOB;
//	motors[3].pin_en = GPIO_PIN_6;
//	motors[3].port_dir = GPIOB;
//	motors[3].pin_dir = GPIO_PIN_4;
//	motors[3].port_step = GPIOB;
//	motors[3].pin_step = GPIO_PIN_5;
//	motors[3].port_uart = GPIOB;
//	motors[3].pin_uart = GPIO_PIN_3;
//	motors[3].uart_index = 0;
//
//	// Motor 4
//	motors[4].targetSteps = 0;
//	motors[4].currentSteps = 0;
//	motors[4].speed = 0;
//	motors[4].timsCounter = 0;
//	motors[4].direction = 1;
//	motors[4].isActive = 0;
//
//	motors[4].port_en = GPIOD;
//	motors[4].pin_en = GPIO_PIN_7;
//	motors[4].port_dir = GPIOD;
//	motors[4].pin_dir = GPIO_PIN_5;
//	motors[4].port_step = GPIOD;
//	motors[4].pin_step = GPIO_PIN_6;
//	motors[4].port_uart = GPIOD;
//	motors[4].pin_uart = GPIO_PIN_4;
//	motors[4].uart_index = 0;
//
//	// Motor 5
//	motors[5].targetSteps = 0;
//	motors[5].currentSteps = 0;
//	motors[5].speed = 0;
//	motors[5].timsCounter = 0;
//	motors[5].direction = 1;
//	motors[5].isActive = 0;
//
//	motors[5].port_en = GPIOD;
//	motors[5].pin_en = GPIO_PIN_3;
//	motors[5].port_dir = GPIOD;
//	motors[5].pin_dir = GPIO_PIN_1;
//	motors[5].port_step = GPIOD;
//	motors[5].pin_step = GPIO_PIN_2;
//	motors[5].port_uart = GPIOD;
//	motors[5].pin_uart = GPIO_PIN_0;
//	motors[5].uart_index = 0;
}

// 初始化电机驱动
void motors_driver_setup(void) {
	for (size_t i = 0; i < NUM_MOTORS; ++i) {
		// 初始化电机的 UART 端口
		SoftUartInit(motors[i].uart_index, motors[i].port_uart, motors[i].pin_uart, NULL, 0);

		setRegisterGCONF(motors[i].uart_index);
		setRegisterIHOLDIRUN(motors[i].uart_index);
		setRegisterCOOLCONF(motors[i].uart_index);
		setRegisterCHOPCONF(motors[i].uart_index);

		setRegisterTCOOLTHRS(motors[i].uart_index);
		setRegisterSGTHRS(motors[i].uart_index);

	}
}

// 启动特定编号的电机
void TMC2209_startStepperMotor(uint8_t motorIndex, uint32_t steps, uint32_t speed, int8_t direction) {
	if (motorIndex < NUM_MOTORS) {
		StepperMotorControl *motor = &motors[motorIndex];
		motor->targetSteps = steps;
		motor->currentSteps = 0;
		motor->speed = (speed > 100) ? 100 : ((speed < 0) ? 0 : speed); // 约束 speed 在 [0, 100] 范围内
		motor->timsCounter = 100 * SPEED_STEP_DIVISION / motor->speed;
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
	for (uint8_t i = 0; i < NUM_MOTORS; ++i) {
		// 检查电机状态
		StepperMotorControl *motor = &motors[i];
		if (motor->speed == 0 || !motor->isActive) {
			continue;
		}

		// 检查是否达到目标步数
		if (motor->currentSteps >= motor->targetSteps) {
			motor->isActive = 0;
			continue;
		}

		// 检查是否应该发送脉冲
		if (motor->timsCounter == 0) {
			// 发送脉冲信号
			HAL_GPIO_TogglePin(motor->port_step, motor->pin_step);

			// 更新步数计数器
			motor->currentSteps += 1;
			motor->timsCounter = 100 * SPEED_STEP_DIVISION / motor->speed;
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

// TMC2209 GCONF 全局配置寄存器初始化
// i_scale_analog = 1，启用外部电压参考VREF
void TMC2209_setGconf(uint8_t instance) {
	const uint8_t msg_enable[] = { 0x05, 0x00, 0x80, 0x00, 0x00, 0x10, 0xC1, 0x7F };
//	const uint8_t msg_enable[] = { 0x05, 0x00, 0x80, 0x00, 0x00, 0x00, 0x83, 0x00 };
	TMC2209_sendRawMessage(instance, msg_enable, sizeof(msg_enable));
	HAL_Delay(10);
}

// TMC2209串口速度驱动
// 扭矩较小，暂不清楚原因，需要关闭stallguard
void TMC2209_moveAtVelocity(uint8_t instance, uint32_t speed) {
	// 停转信号
	uint8_t msg_init[8] = { 0x05, 0x00, 0xA2, 0x00, 0x00, 0x00, 0x00, 0x0E };
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



