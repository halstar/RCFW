################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Rcfw/Src/battery_check.c \
../Rcfw/Src/bluetooth_control.c \
../Rcfw/Src/chrono.c \
../Rcfw/Src/circular_buffer.c \
../Rcfw/Src/console.c \
../Rcfw/Src/drive.c \
../Rcfw/Src/encoder.c \
../Rcfw/Src/led.c \
../Rcfw/Src/log.c \
../Rcfw/Src/master_control.c \
../Rcfw/Src/motor.c \
../Rcfw/Src/pid.c \
../Rcfw/Src/rcfw.c \
../Rcfw/Src/string_fifo.c \
../Rcfw/Src/utils.c \
../Rcfw/Src/wheel.c 

OBJS += \
./Rcfw/Src/battery_check.o \
./Rcfw/Src/bluetooth_control.o \
./Rcfw/Src/chrono.o \
./Rcfw/Src/circular_buffer.o \
./Rcfw/Src/console.o \
./Rcfw/Src/drive.o \
./Rcfw/Src/encoder.o \
./Rcfw/Src/led.o \
./Rcfw/Src/log.o \
./Rcfw/Src/master_control.o \
./Rcfw/Src/motor.o \
./Rcfw/Src/pid.o \
./Rcfw/Src/rcfw.o \
./Rcfw/Src/string_fifo.o \
./Rcfw/Src/utils.o \
./Rcfw/Src/wheel.o 

C_DEPS += \
./Rcfw/Src/battery_check.d \
./Rcfw/Src/bluetooth_control.d \
./Rcfw/Src/chrono.d \
./Rcfw/Src/circular_buffer.d \
./Rcfw/Src/console.d \
./Rcfw/Src/drive.d \
./Rcfw/Src/encoder.d \
./Rcfw/Src/led.d \
./Rcfw/Src/log.d \
./Rcfw/Src/master_control.d \
./Rcfw/Src/motor.d \
./Rcfw/Src/pid.d \
./Rcfw/Src/rcfw.d \
./Rcfw/Src/string_fifo.d \
./Rcfw/Src/utils.d \
./Rcfw/Src/wheel.d 


# Each subdirectory must supply rules for building sources it contributes
Rcfw/Src/%.o: ../Rcfw/Src/%.c Rcfw/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32F103xE -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I../Rcfw/Inc -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

