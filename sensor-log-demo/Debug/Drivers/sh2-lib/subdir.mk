################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/sh2-lib/euler.c \
../Drivers/sh2-lib/sh2.c \
../Drivers/sh2-lib/sh2_SensorValue.c \
../Drivers/sh2-lib/sh2_util.c \
../Drivers/sh2-lib/shtp.c 

OBJS += \
./Drivers/sh2-lib/euler.o \
./Drivers/sh2-lib/sh2.o \
./Drivers/sh2-lib/sh2_SensorValue.o \
./Drivers/sh2-lib/sh2_util.o \
./Drivers/sh2-lib/shtp.o 

C_DEPS += \
./Drivers/sh2-lib/euler.d \
./Drivers/sh2-lib/sh2.d \
./Drivers/sh2-lib/sh2_SensorValue.d \
./Drivers/sh2-lib/sh2_util.d \
./Drivers/sh2-lib/shtp.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/sh2-lib/%.o Drivers/sh2-lib/%.su Drivers/sh2-lib/%.cyclo: ../Drivers/sh2-lib/%.c Drivers/sh2-lib/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DCORE_CM4 -DUSE_HAL_DRIVER -DSTM32WLE5xx -DUSE_FULL_LL_DRIVER -c -I"/Users/michaelshipman/Developer/ENEE408A/ENEE408A/bno085-demo/Drivers/sh2-lib" -I../Core/Inc -I../Drivers/STM32WLxx_HAL_Driver/Inc -I../Drivers/STM32WLxx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32WLxx/Include -I../Drivers/CMSIS/Include -I../FATFS/Target -I../FATFS/App -I../Middlewares/Third_Party/FatFs/src -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Drivers-2f-sh2-2d-lib

clean-Drivers-2f-sh2-2d-lib:
	-$(RM) ./Drivers/sh2-lib/euler.cyclo ./Drivers/sh2-lib/euler.d ./Drivers/sh2-lib/euler.o ./Drivers/sh2-lib/euler.su ./Drivers/sh2-lib/sh2.cyclo ./Drivers/sh2-lib/sh2.d ./Drivers/sh2-lib/sh2.o ./Drivers/sh2-lib/sh2.su ./Drivers/sh2-lib/sh2_SensorValue.cyclo ./Drivers/sh2-lib/sh2_SensorValue.d ./Drivers/sh2-lib/sh2_SensorValue.o ./Drivers/sh2-lib/sh2_SensorValue.su ./Drivers/sh2-lib/sh2_util.cyclo ./Drivers/sh2-lib/sh2_util.d ./Drivers/sh2-lib/sh2_util.o ./Drivers/sh2-lib/sh2_util.su ./Drivers/sh2-lib/shtp.cyclo ./Drivers/sh2-lib/shtp.d ./Drivers/sh2-lib/shtp.o ./Drivers/sh2-lib/shtp.su

.PHONY: clean-Drivers-2f-sh2-2d-lib

