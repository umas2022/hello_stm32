#ifndef __FLASH_H
#define __FLASH_H

#include "stm32f1xx_hal.h"

// 定义用户Flash存储区起始地址
#define FLASH_USER_START_ADDR   0x0800FC00   // 最后一页的起始地址

// 函数声明
HAL_StatusTypeDef Flash_Write_Number(uint32_t Address, uint32_t number);
uint32_t Flash_Read_Number(uint32_t Address);
HAL_StatusTypeDef Flash_Init(void);

#endif /* __FLASH_H */
