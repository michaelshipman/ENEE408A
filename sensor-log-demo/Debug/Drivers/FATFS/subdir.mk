################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/FATFS/app_fatfs.c \
../Drivers/FATFS/diskio.c \
../Drivers/FATFS/ff.c \
../Drivers/FATFS/ff_gen_drv.c \
../Drivers/FATFS/syscall.c \
../Drivers/FATFS/user_diskio.c \
../Drivers/FATFS/user_diskio_spi.c 

OBJS += \
./Drivers/FATFS/app_fatfs.o \
./Drivers/FATFS/diskio.o \
./Drivers/FATFS/ff.o \
./Drivers/FATFS/ff_gen_drv.o \
./Drivers/FATFS/syscall.o \
./Drivers/FATFS/user_diskio.o \
./Drivers/FATFS/user_diskio_spi.o 

C_DEPS += \
./Drivers/FATFS/app_fatfs.d \
./Drivers/FATFS/diskio.d \
./Drivers/FATFS/ff.d \
./Drivers/FATFS/ff_gen_drv.d \
./Drivers/FATFS/syscall.d \
./Drivers/FATFS/user_diskio.d \
./Drivers/FATFS/user_diskio_spi.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/FATFS/%.o Drivers/FATFS/%.su Drivers/FATFS/%.cyclo: ../Drivers/FATFS/%.c Drivers/FATFS/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DCORE_CM4 -DUSE_HAL_DRIVER -DSTM32WLE5xx -DUSE_FULL_LL_DRIVER -c -I"/Users/michaelshipman/Developer/ENEE408A/bno085-demo/Drivers/FATFS" -I"/Users/michaelshipman/Developer/ENEE408A/bno085-demo/Drivers/sh2-lib" -I../Core/Inc -I../Drivers/STM32WLxx_HAL_Driver/Inc -I../Drivers/STM32WLxx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32WLxx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Drivers-2f-FATFS

clean-Drivers-2f-FATFS:
	-$(RM) ./Drivers/FATFS/app_fatfs.cyclo ./Drivers/FATFS/app_fatfs.d ./Drivers/FATFS/app_fatfs.o ./Drivers/FATFS/app_fatfs.su ./Drivers/FATFS/diskio.cyclo ./Drivers/FATFS/diskio.d ./Drivers/FATFS/diskio.o ./Drivers/FATFS/diskio.su ./Drivers/FATFS/ff.cyclo ./Drivers/FATFS/ff.d ./Drivers/FATFS/ff.o ./Drivers/FATFS/ff.su ./Drivers/FATFS/ff_gen_drv.cyclo ./Drivers/FATFS/ff_gen_drv.d ./Drivers/FATFS/ff_gen_drv.o ./Drivers/FATFS/ff_gen_drv.su ./Drivers/FATFS/syscall.cyclo ./Drivers/FATFS/syscall.d ./Drivers/FATFS/syscall.o ./Drivers/FATFS/syscall.su ./Drivers/FATFS/user_diskio.cyclo ./Drivers/FATFS/user_diskio.d ./Drivers/FATFS/user_diskio.o ./Drivers/FATFS/user_diskio.su ./Drivers/FATFS/user_diskio_spi.cyclo ./Drivers/FATFS/user_diskio_spi.d ./Drivers/FATFS/user_diskio_spi.o ./Drivers/FATFS/user_diskio_spi.su

.PHONY: clean-Drivers-2f-FATFS

