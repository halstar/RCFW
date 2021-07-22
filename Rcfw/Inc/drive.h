#ifndef __DRIVE_H
#define __DRIVE_H

#include "stm32f1xx_hal.h"

#include "bluetooth_control.h"

void DRIVE_init  (TIM_HandleTypeDef      *p_pwmTimerHandle);
void DRIVE_update(T_BLUETOOTH_ControlData *p_bluetoothData );

#endif /* __DRIVE_H */
