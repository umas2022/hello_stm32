#include "TMC2209.h"
#include <stdint.h>
#include <string.h>

// 发送原始消息
void TMC2209_sendRawMessage(UART_HandleTypeDef *huart, const uint8_t *message,
		size_t length) {
	HAL_UART_Transmit(huart, (uint8_t*) message, length, HAL_MAX_DELAY);
}

// TMC2209初始化
void TMC2209_Init(UART_HandleTypeDef *huart) {
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
		TMC2209_sendRawMessage(huart, msg_setup[i], sizeof(msg_setup[i]));
		HAL_Delay(10); // 短暂延迟以确保每条消息都被正确接收
	}
}

// TMC2209设置电机运行电流100%
void TMC2209_setRunCurrent_100(UART_HandleTypeDef *huart) {
	const uint8_t msg_current[] = { 0x05, 0x00, 0x90, 0x00, 0x01, 0x1F, 0x00,
			0xE7 };
	TMC2209_sendRawMessage(huart, msg_current, sizeof(msg_current));
	HAL_Delay(10);
}

// TMC2209启用coolstep
void TMC2209_enableCoolStep(UART_HandleTypeDef *huart) {
	const uint8_t msg_coolstep[] = { 0x05, 0x00, 0xC2, 0x00, 0x00, 0x80, 0x01,
			0xD9 };
	TMC2209_sendRawMessage(huart, msg_coolstep, sizeof(msg_coolstep));
	HAL_Delay(10);
}

// TMC2209启动
void TMC2209_enable(UART_HandleTypeDef *huart) {
	const uint8_t msg_enable[] = { 0x05, 0x00, 0xEC, 0x10, 0x01, 0x00, 0x53,
			0x97 };
	TMC2209_sendRawMessage(huart, msg_enable, sizeof(msg_enable));
	HAL_Delay(10);
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

// TMC2209设置速度
void TMC2209_moveAtVelocity(UART_HandleTypeDef *huart, uint32_t speed) {
	uint8_t msg_move[8] = { 0x05, 0x00, 0xA2, 0x00, 0x00, 0x00, 0x00, 0x00 };

	// 设置速度值（大端模式）
	msg_move[3] = (speed >> 24) & 0xFF;  // 速度的高字节
	msg_move[4] = (speed >> 16) & 0xFF;  // 速度的次高字节
	msg_move[5] = (speed >> 8) & 0xFF;   // 速度的次低字节
	msg_move[6] = speed & 0xFF;          // 速度的低字节
	uint8_t crc = calculate_crc(msg_move, sizeof(msg_move));
	msg_move[7] = crc;  // 将CRC值存入消息的最后一个字节

	// 发送消息
	TMC2209_sendRawMessage(huart, msg_move, sizeof(msg_move));
	HAL_Delay(10);
}
