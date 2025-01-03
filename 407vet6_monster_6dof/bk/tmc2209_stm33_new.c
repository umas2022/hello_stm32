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
	writeRegister(uart_instance, ADDRESS_GCONF, register_gconf.bytes);
}

// 设置寄存器IHOLD_IRUN
// irun>=16时堵转检测数值不稳定
RegisterCurrent register_iholdirun;
void setRegisterIHOLDIRUN(uint8_t uart_instance) {
	register_iholdirun.bytes = 0;
	// 取值0-31
	register_iholdirun.ihold = 16;
	register_iholdirun.irun = 15;
	writeRegister(uart_instance, ADDRESS_IHOLD_IRUN, register_iholdirun.bytes);

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
	writeRegister(uart_instance, ADDRESS_COOLCONF, register_coolconf.bytes);
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
	writeRegister(uart_instance, ADDRESS_CHOPCONF, register_chopconf.bytes);
}

// 寄存器TCOOLTHRS
// CoolStep 和 StallGuard 功能的下限速度阈值，当电机的速度超过这个阈值时，CoolStep 和 StallGuard 功能将被启用
RegisterTCOOLTHRS register_tcoolthrs;
void setRegisterTCOOLTHRS(uint8_t uart_instance){
	register_tcoolthrs.bytes = 0;
	register_tcoolthrs.threshold = 0xFFFF; // 设置下限速度阈值
//	register_tcoolthrs.threshold = 0xFFFFF; // 设置下限速度阈值
    writeRegister(uart_instance, ADDRESS_TCOOLTHRS, register_tcoolthrs.bytes);
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
    writeRegister(uart_instance, ADDRESS_SGTHRS, register_sgthrs.bytes);
}

// 写寄存器
void writeRegister(uint8_t uart_instance, uint8_t register_address, uint32_t data) {

	WriteReadReplyDatagram write_datagram;
	write_datagram.bytes = 0;
	write_datagram.sync = SYNC;
	write_datagram.serial_address = 0;
	write_datagram.register_address = register_address;
	write_datagram.rw = RW_WRITE;
	write_datagram.data = reverseData(data);

	uint8_t buffer_crc[WRITE_READ_REPLY_DATAGRAM_SIZE];
	memcpy(buffer_crc, &write_datagram.bytes, WRITE_READ_REPLY_DATAGRAM_SIZE);
	write_datagram.crc = calculateCrc(buffer_crc, WRITE_READ_REPLY_DATAGRAM_SIZE);

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
uint8_t calculateCrc(unsigned char *datagram, size_t length) {
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
	void motorsDefine() {
	    // 初始化每个电机
	    for (uint8_t i = 0; i < NUM_MOTORS; ++i) {
	        motors[i].motorID = 0;
	        motors[i].action.targetSteps = 0;
	        motors[i].action.currentSteps = 0;
	        motors[i].action.speed = 0;
	        motors[i].action.timsCounter = 0;
	        motors[i].action.direction = 1;
	        motors[i].action.isActive = 0;
	        motors[i].pins.port_diag = NULL;
	        motors[i].pins.pin_diag = 0;
	    }

	    // Motor 0
	    motors[0].pins.port_en = GPIOC;
	    motors[0].pins.pin_en = GPIO_PIN_15;
	    motors[0].pins.port_dir = GPIOC;
	    motors[0].pins.pin_dir = GPIO_PIN_13;
	    motors[0].pins.port_step = GPIOC;
	    motors[0].pins.pin_step = GPIO_PIN_14;
	    motors[0].pins.port_uart = GPIOE;
	    motors[0].pins.pin_uart = GPIO_PIN_6;
//
//	    // Motor 1
//	    motors[1].pins.port_en = GPIOC;
//	    motors[1].pins.pin_en = GPIO_PIN_15;
//	    motors[1].pins.port_dir = GPIOE;
//	    motors[1].pins.pin_dir = GPIO_PIN_4;
//	    motors[1].pins.port_step = GPIOE;
//	    motors[1].pins.pin_step = GPIO_PIN_5;
//	    motors[1].pins.port_uart = GPIOE;
//	    motors[1].pins.pin_uart = GPIO_PIN_3;
//
//	    // Motor 2
//	    motors[2].pins.port_en = GPIOE;
//	    motors[2].pins.pin_en = GPIO_PIN_2;
//	    motors[2].pins.port_dir = GPIOE;
//	    motors[2].pins.pin_dir = GPIO_PIN_0;
//	    motors[2].pins.port_step = GPIOE;
//	    motors[2].pins.pin_step = GPIO_PIN_1;
//	    motors[2].pins.port_uart = GPIOB;
//	    motors[2].pins.pin_uart = GPIO_PIN_7;
//
//	    // Motor 3
//	    motors[3].pins.port_en = GPIOB;
//	    motors[3].pins.pin_en = GPIO_PIN_6;
//	    motors[3].pins.port_dir = GPIOB;
//	    motors[3].pins.pin_dir = GPIO_PIN_4;
//	    motors[3].pins.port_step = GPIOB;
//	    motors[3].pins.pin_step = GPIO_PIN_5;
//	    motors[3].pins.port_uart = GPIOB;
//	    motors[3].pins.pin_uart = GPIO_PIN_3;
//
//	    // Motor 4
//	    motors[4].pins.port_en = GPIOD;
//	    motors[4].pins.pin_en = GPIO_PIN_7;
//	    motors[4].pins.port_dir = GPIOD;
//	    motors[4].pins.pin_dir = GPIO_PIN_5;
//	    motors[4].pins.port_step = GPIOD;
//	    motors[4].pins.pin_step = GPIO_PIN_6;
//	    motors[4].pins.port_uart = GPIOD;
//	    motors[4].pins.pin_uart = GPIO_PIN_4;
//
//	    // Motor 5
//	    motors[5].pins.port_en = GPIOD;
//	    motors[5].pins.pin_en = GPIO_PIN_3;
//	    motors[5].pins.port_dir = GPIOD;
//	    motors[5].pins.pin_dir = GPIO_PIN_1;
//	    motors[5].pins.port_step = GPIOD;
//	    motors[5].pins.pin_step = GPIO_PIN_2;
//	    motors[5].pins.port_uart = GPIOD;
//	    motors[5].pins.pin_uart = GPIO_PIN_0;

}

// 初始化电机驱动
void motorsDriverSetup(void) {
	for (size_t i = 0; i < NUM_MOTORS; ++i) {
		// 初始化电机的 UART 端口

		SoftUartInit(motors[i].motorID, motors[i].pins.port_uart, motors[i].pins.pin_uart, NULL, 0);

		setRegisterGCONF(motors[i].motorID);
		setRegisterIHOLDIRUN(motors[i].motorID);
		setRegisterCOOLCONF(motors[i].motorID);
		setRegisterCHOPCONF(motors[i].motorID);

		setRegisterTCOOLTHRS(motors[i].motorID);
		setRegisterSGTHRS(motors[i].motorID);

//		SoftUartInit(motors[i].uart_index, motors[i].port_uart, motors[i].pin_uart, NULL, 0);
//
//		setRegisterGCONF(motors[i].uart_index);
//		setRegisterIHOLDIRUN(motors[i].uart_index);
//		setRegisterCOOLCONF(motors[i].uart_index);
//		setRegisterCHOPCONF(motors[i].uart_index);
//
//		setRegisterTCOOLTHRS(motors[i].uart_index);
//		setRegisterSGTHRS(motors[i].uart_index);

	}
}

// 启动特定编号的电机
void motorsActionQueuePush(uint8_t motorIndex, uint32_t steps, uint32_t speed, int8_t direction) {
	if (motorIndex < NUM_MOTORS) {
		StepperMotorControl motor = motors[motorIndex];
		motor.action.targetSteps = steps;
		motor.action.currentSteps = 0;
		motor.action.speed = (speed > 100) ? 100 : ((speed < 0) ? 0 : speed); // 约束 speed 在 [0, 100] 范围内
		motor.action.timsCounter = 100 * SPEED_STEP_DIVISION / motor.action.speed;
		motor.action.direction = direction;
		motor.action.isActive = 1;

		// 设置使能信号
		HAL_GPIO_WritePin(motor.pins.port_en, motor.pins.pin_en, GPIO_PIN_RESET);

		// 设置初始方向
		HAL_GPIO_WritePin(motor.pins.port_dir, motor.pins.pin_dir, (direction > 0) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	}
}

// 定时器周期结束回调函数
void motorsStepHandler(void) {
	for (uint8_t i = 0; i < NUM_MOTORS; ++i) {
		// 检查电机状态
		StepperMotorControl motor = motors[i];
		if (motor.action.speed == 0 || !motor.action.isActive) {
			continue;
		}

		// 检查是否达到目标步数
		if (motor.action.currentSteps >= motor.action.targetSteps) {
			motor.action.isActive = 0;
			continue;
		}

		// 检查是否应该发送脉冲
		if (motor.action.timsCounter == 0) {
			// 发送脉冲信号
			HAL_GPIO_TogglePin(motor.pins.port_step, motor.pins.pin_step);

			// 更新步数计数器
			motor.action.currentSteps += 1;
			motor.action.timsCounter = 100 * SPEED_STEP_DIVISION / motor.action.speed;
		}
		motor.action.timsCounter -= 1;
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


