#include "led.h"

#include "stm32f1xx_hal.h"
#include "main.h"

void LED_toggle(void)
{
  HAL_GPIO_TogglePin(BLUE_LED_GPIO_Port, BLUE_LED_Pin);
}
