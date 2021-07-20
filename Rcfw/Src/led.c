#include "led.h"

#include "stm32f1xx_hal.h"
#include "main.h"
#include "log.h"

static uint32_t LED_mode = LED_MODE_BLINK_SLOW;

void LED_setMode(uint32_t mode)
{
  LOG_info("Setting LED mode to %u", mode);

  LED_mode = mode;

  return;
}

void LED_update(void)
{
  static uint32_t slowBlinkCounter = 0;

  LOG_debug("Updating LED");

  switch (LED_mode)
  {
    case LED_MODE_FORCED_OFF:
      HAL_GPIO_WritePin(BLUE_LED_GPIO_Port, BLUE_LED_Pin, GPIO_PIN_SET);
      break;

    case LED_MODE_FORCED_ON:
      HAL_GPIO_WritePin(BLUE_LED_GPIO_Port, BLUE_LED_Pin, GPIO_PIN_RESET);
      break;

    case LED_MODE_BLINK_SLOW:
      slowBlinkCounter++;
      if (slowBlinkCounter % 4 == 0)
      {
        HAL_GPIO_TogglePin(BLUE_LED_GPIO_Port, BLUE_LED_Pin);
      }
      else
      {
        ; /* Nothing to do */
      }
      break;

    case LED_MODE_BLINK_FAST:
      HAL_GPIO_TogglePin(BLUE_LED_GPIO_Port, BLUE_LED_Pin);
      break;

    default:
      ; /* Nothing to do */
      break;
  }

  return;
}
