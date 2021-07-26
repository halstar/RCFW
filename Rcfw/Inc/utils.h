#ifndef __UTILS_H
#define __UTILS_H

#include <stdbool.h>

#include "stm32f1xx_hal.h"

#define UTILS_MAX(a, b) ((a) > (b) ? a : b)
#define UTILS_MIN(a, b) ((a) < (b) ? a : b)

void    UTILS_init         (TIM_HandleTypeDef *htim );
void    UTILS_delayUs      (uint16_t           delay);
int32_t UTILS_clampIntValue(int32_t p_value, int32_t p_minValue, int32_t p_maxValue, bool p_clampToNearest, int32_t p_clampValue);

#endif /* __UTILS_H */
