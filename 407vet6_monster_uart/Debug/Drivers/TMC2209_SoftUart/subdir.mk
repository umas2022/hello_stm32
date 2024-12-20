################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/TMC2209_SoftUart/softuart.c \
../Drivers/TMC2209_SoftUart/tmc2209_uart.c 

OBJS += \
./Drivers/TMC2209_SoftUart/softuart.o \
./Drivers/TMC2209_SoftUart/tmc2209_uart.o 

C_DEPS += \
./Drivers/TMC2209_SoftUart/softuart.d \
./Drivers/TMC2209_SoftUart/tmc2209_uart.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/TMC2209_SoftUart/%.o Drivers/TMC2209_SoftUart/%.su Drivers/TMC2209_SoftUart/%.cyclo: ../Drivers/TMC2209_SoftUart/%.c Drivers/TMC2209_SoftUart/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"D:/ws-stm32/407vet6_monster_uart/Drivers/TMC2209_SoftUart" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-TMC2209_SoftUart

clean-Drivers-2f-TMC2209_SoftUart:
	-$(RM) ./Drivers/TMC2209_SoftUart/softuart.cyclo ./Drivers/TMC2209_SoftUart/softuart.d ./Drivers/TMC2209_SoftUart/softuart.o ./Drivers/TMC2209_SoftUart/softuart.su ./Drivers/TMC2209_SoftUart/tmc2209_uart.cyclo ./Drivers/TMC2209_SoftUart/tmc2209_uart.d ./Drivers/TMC2209_SoftUart/tmc2209_uart.o ./Drivers/TMC2209_SoftUart/tmc2209_uart.su

.PHONY: clean-Drivers-2f-TMC2209_SoftUart

