#ifndef __DRIVE_H
#define __DRIVE_H

#include "stm32f1xx_hal.h"

#include "bluetooth_control.h"

#define DRV_MAXIMUM_SPEED 60

void DRV_init(TIM_HandleTypeDef *p_pwmTimerHandle,
              TIM_HandleTypeDef *p_rearLeftEncoderTimerHandle,
              TIM_HandleTypeDef *p_rearRightEncoderTimerHandle,
              TIM_HandleTypeDef *p_frontRightEncoderTimerHandle,
              TIM_HandleTypeDef *p_frontLeftEncoderTimerHandle);

void DRV_updateOnBluetooth(T_BLU_Data        *p_bluetoothData     );
void DRV_updateOnEncoder  (TIM_HandleTypeDef *p_encoderTimerHandle);

#endif /* __DRIVE_H */
