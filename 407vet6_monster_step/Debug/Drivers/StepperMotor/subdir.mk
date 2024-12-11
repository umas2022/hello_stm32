################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/StepperMotor/stepper_motor.c 

OBJS += \
./Drivers/StepperMotor/stepper_motor.o 

C_DEPS += \
./Drivers/StepperMotor/stepper_motor.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/StepperMotor/%.o Drivers/StepperMotor/%.su Drivers/StepperMotor/%.cyclo: ../Drivers/StepperMotor/%.c Drivers/StepperMotor/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"D:/ws-stm32/407vet6_monster_step/Drivers/StepperMotor" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-StepperMotor

clean-Drivers-2f-StepperMotor:
	-$(RM) ./Drivers/StepperMotor/stepper_motor.cyclo ./Drivers/StepperMotor/stepper_motor.d ./Drivers/StepperMotor/stepper_motor.o ./Drivers/StepperMotor/stepper_motor.su

.PHONY: clean-Drivers-2f-StepperMotor

