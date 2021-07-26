#ifndef __DRIVE_H
#define __DRIVE_H

#include "stm32f1xx_hal.h"

#include "bluetooth_control.h"

#define DRV_MAXIMUM_SPEED 60

void DRV_init  (TIM_HandleTypeDef *p_pwmTimerHandle);
void DRV_update(T_BLU_Data        *p_bluetoothData );

#endif /* __DRIVE_H */
