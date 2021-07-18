#ifndef __UTILS_H
#define __UTILS_H

#include "stm32f1xx_hal.h"

void UTILS_delayUsInit(TIM_HandleTypeDef *htim);
void UTILS_delayUs(uint16_t delay);

#endif /* __UTILS_H */
