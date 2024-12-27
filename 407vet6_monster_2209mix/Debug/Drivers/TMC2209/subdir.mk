################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/TMC2209/softuart.c \
../Drivers/TMC2209/tmc2209_stm32.c 

C_DEPS += \
./Drivers/TMC2209/softuart.d \
./Drivers/TMC2209/tmc2209_stm32.d 

OBJS += \
./Drivers/TMC2209/softuart.o \
./Drivers/TMC2209/tmc2209_stm32.o 


# Each subdirectory must supply rules for building sources it contributes
Drivers/TMC2209/%.o Drivers/TMC2209/%.su Drivers/TMC2209/%.cyclo: ../Drivers/TMC2209/%.c Drivers/TMC2209/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"D:/ws-stm32/407vet6_monster_2209mix/Drivers/TMC2209" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-TMC2209

clean-Drivers-2f-TMC2209:
	-$(RM) ./Drivers/TMC2209/softuart.cyclo ./Drivers/TMC2209/softuart.d ./Drivers/TMC2209/softuart.o ./Drivers/TMC2209/softuart.su ./Drivers/TMC2209/tmc2209_stm32.cyclo ./Drivers/TMC2209/tmc2209_stm32.d ./Drivers/TMC2209/tmc2209_stm32.o ./Drivers/TMC2209/tmc2209_stm32.su

.PHONY: clean-Drivers-2f-TMC2209

