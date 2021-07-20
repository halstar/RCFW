#ifndef __DRIVE_H
#define __DRIVE_H

#include "stm32f1xx_hal.h"

#include "bluetooth_control.h"

void DRIVE_init  (TIM_HandleTypeDef      *htim);
void DRIVE_update(BLUETOOTH_CONTROL_DATA *data);

#endif /* __DRIVE_H */
