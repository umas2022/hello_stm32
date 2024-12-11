################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/TMC2209/tmc2209.c 

C_DEPS += \
./Drivers/TMC2209/tmc2209.d 

OBJS += \
./Drivers/TMC2209/tmc2209.o 


# Each subdirectory must supply rules for building sources it contributes
Drivers/TMC2209/%.o Drivers/TMC2209/%.su Drivers/TMC2209/%.cyclo: ../Drivers/TMC2209/%.c Drivers/TMC2209/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I"D:/ws-stm32/103c8t6_tmc2209_hello/Drivers/TMC2209" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Drivers-2f-TMC2209

clean-Drivers-2f-TMC2209:
	-$(RM) ./Drivers/TMC2209/tmc2209.cyclo ./Drivers/TMC2209/tmc2209.d ./Drivers/TMC2209/tmc2209.o ./Drivers/TMC2209/tmc2209.su

.PHONY: clean-Drivers-2f-TMC2209

