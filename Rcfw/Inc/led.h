#ifndef __LED_H
#define __LED_H

#include "stm32f1xx_hal.h"

#define LED_MODE_FORCED_OFF 0
#define LED_MODE_FORCED_ON  1
#define LED_MODE_BLINK_SLOW 2
#define LED_MODE_BLINK_FAST 3

void LED_setMode(uint32_t mode);
void LED_update (void         );

#endif /* __LED_H */
