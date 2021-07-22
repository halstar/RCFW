#ifndef __BATTERY_CHECK_H
#define __BATTERY_CHECK_H

#include "stm32f1xx_hal.h"

void BATTERY_CHECK_init  (ADC_HandleTypeDef *p_adcHandle, RTC_HandleTypeDef *p_rtcHandle);
void BATTERY_CHECK_update(void                                                          );

#endif /* __BATTERY_CHECK_H */
