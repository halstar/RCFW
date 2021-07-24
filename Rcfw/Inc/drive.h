#ifndef __DRIVE_H
#define __DRIVE_H

#include "stm32f1xx_hal.h"

#include "bluetooth_control.h"

#define DRIVE_MAXIMUM_SPEED 60

void DRIVE_init  (TIM_HandleTypeDef        *p_pwmTimerHandle);
void DRIVE_update(T_BLUETOOTH_CONTROL_Data *p_bluetoothData );

#endif /* __DRIVE_H */
