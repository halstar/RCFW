#ifndef __UTILS_H
#define __UTILS_H

#include <stdbool.h>

#include "stm32f1xx_hal.h"

void UTILS_init          (TIM_HandleTypeDef *htim );
void UTILS_delayUs       (uint16_t           delay);
void UTILS_clampUIntValue(uint32_t *p_value, uint32_t p_minValue, uint32_t p_maxValue, bool p_clampToNearest, uint32_t p_clampValue);

#endif /* __UTILS_H */
