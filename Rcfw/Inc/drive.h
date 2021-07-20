#ifndef __DRIVE_H
#define __DRIVE_H

#include "stm32f1xx_hal.h"

void DRIVE_init  (TIM_HandleTypeDef *htim);
void DRIVE_update(void);

#endif /* __DRIVE_H */
