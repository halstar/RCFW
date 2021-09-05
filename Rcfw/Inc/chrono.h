#ifndef __CHRONO_H
#define __CHRONO_H

#include <stdbool.h>

#include "stm32f1xx_hal.h"

void     CHR_init     (TIM_HandleTypeDef *p_usTimerHandle);
void     CHR_start    (void);
void     CHR_stop     (void);
void     CHR_reset    (void);
uint32_t CHR_getTimeUs(void);
void     CHR_overflow (void);

#endif /* __CHRONO_H */
