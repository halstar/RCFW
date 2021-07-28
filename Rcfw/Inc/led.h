#ifndef __LED_H
#define __LED_H

#include "stm32f1xx_hal.h"

typedef enum
{
  LED_MODE_FORCED_OFF = 0,
  LED_MODE_FORCED_ON,
  LED_MODE_BLINK_SLOW,
  LED_MODE_BLINK_MEDIUM,
  LED_MODE_BLINK_FAST
} T_LED_MODE;

void       LED_setMode(T_LED_MODE p_mode);
T_LED_MODE LED_getMode(void             );
void       LED_update (void             );

#endif /* __LED_H */
