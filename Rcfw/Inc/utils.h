#ifndef __UTILS_H
#define __UTILS_H

#include <stdbool.h>

#include "stm32f1xx_hal.h"

#define UTI_MAX(a, b) ((a) > (b) ? a : b)
#define UTI_MIN(a, b) ((a) < (b) ? a : b)

void     UTI_init                (TIM_HandleTypeDef *p_delayTimerHandle);
void     UTI_delayUs             (uint16_t           p_delay           );
void     UTI_resetRtcDate        (RTC_DateTypeDef   *p_date            );
void     UTI_resetRtcTime        (RTC_TimeTypeDef   *p_time            );
uint32_t UTI_turnRtcTimeToSeconds(RTC_TimeTypeDef   *p_time            );
int32_t  UTI_clampIntValue       (int32_t p_value, int32_t p_minValue, int32_t p_maxValue, bool p_clampToNearest, int32_t p_clampValue);

#endif /* __UTILS_H */
