################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Rcfw/Src/bluetooth_control.c \
../Rcfw/Src/console.c \
../Rcfw/Src/led.c \
../Rcfw/Src/master_control.c \
../Rcfw/Src/utils.c 

OBJS += \
./Rcfw/Src/bluetooth_control.o \
./Rcfw/Src/console.o \
./Rcfw/Src/led.o \
./Rcfw/Src/master_control.o \
./Rcfw/Src/utils.o 

C_DEPS += \
./Rcfw/Src/bluetooth_control.d \
./Rcfw/Src/console.d \
./Rcfw/Src/led.d \
./Rcfw/Src/master_control.d \
./Rcfw/Src/utils.d 


# Each subdirectory must supply rules for building sources it contributes
Rcfw/Src/bluetooth_control.o: ../Rcfw/Src/bluetooth_control.c Rcfw/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xE -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I../Rcfw/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Rcfw/Src/bluetooth_control.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Rcfw/Src/console.o: ../Rcfw/Src/console.c Rcfw/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xE -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I../Rcfw/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Rcfw/Src/console.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Rcfw/Src/led.o: ../Rcfw/Src/led.c Rcfw/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xE -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I../Rcfw/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Rcfw/Src/led.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Rcfw/Src/master_control.o: ../Rcfw/Src/master_control.c Rcfw/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xE -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I../Rcfw/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Rcfw/Src/master_control.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Rcfw/Src/utils.o: ../Rcfw/Src/utils.c Rcfw/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xE -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I../Rcfw/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Rcfw/Src/utils.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

