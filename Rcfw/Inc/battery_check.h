#ifndef __BATTERY_CHECK_H
#define __BATTERY_CHECK_H

#include "stm32f1xx_hal.h"

void BAT_init  (ADC_HandleTypeDef *p_adcHandle );
void BAT_update(uint32_t          *p_voltageInV);

#endif /* __BATTERY_CHECK_H */
