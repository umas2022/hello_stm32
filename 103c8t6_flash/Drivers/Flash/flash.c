#include "flash.h"

HAL_StatusTypeDef Flash_Init(void)
{
    HAL_FLASH_Unlock();
    return HAL_OK;
}

HAL_StatusTypeDef Flash_Write_Number(uint32_t Address, uint32_t number)
{
    HAL_StatusTypeDef status = HAL_OK;

    // 擦除页面
    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t PageError = 0;

    EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.PageAddress = Address & ~(FLASH_PAGE_SIZE - 1);
    EraseInitStruct.NbPages = 1;

    if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK)
    {
        return HAL_ERROR;
    }

    // 写入数值到Flash
    status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, Address, number);

    HAL_FLASH_Lock();
    return status;
}

uint32_t Flash_Read_Number(uint32_t Address)
{
    // 读取小端序的数据
    uint32_t readNumber = *(__IO uint32_t*)Address;

    // 将小端序转换为大端序
    return ((readNumber & 0xFF000000) >> 24) |
           ((readNumber & 0x00FF0000) >> 8)  |
           ((readNumber & 0x0000FF00) << 8)  |
           ((readNumber & 0x000000FF) << 24);
}
