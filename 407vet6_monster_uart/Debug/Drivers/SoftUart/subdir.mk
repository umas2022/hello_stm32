################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/SoftUart/softuart.c 

OBJS += \
./Drivers/SoftUart/softuart.o 

C_DEPS += \
./Drivers/SoftUart/softuart.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/SoftUart/%.o Drivers/SoftUart/%.su Drivers/SoftUart/%.cyclo: ../Drivers/SoftUart/%.c Drivers/SoftUart/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"D:/ws-stm32/407vet6_monster_uart/Drivers/SoftUart" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-SoftUart

clean-Drivers-2f-SoftUart:
	-$(RM) ./Drivers/SoftUart/softuart.cyclo ./Drivers/SoftUart/softuart.d ./Drivers/SoftUart/softuart.o ./Drivers/SoftUart/softuart.su

.PHONY: clean-Drivers-2f-SoftUart

