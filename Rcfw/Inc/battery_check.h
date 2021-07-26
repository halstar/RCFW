#ifndef __BATTERY_CHECK_H
#define __BATTERY_CHECK_H

#include "stm32f1xx_hal.h"

void BAT_init  (ADC_HandleTypeDef *p_adcHandle, RTC_HandleTypeDef *p_rtcHandle);
void BAT_update(void                                                          );

#endif /* __BATTERY_CHECK_H */
