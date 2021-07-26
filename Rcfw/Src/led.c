#include "led.h"

#include "stm32f1xx_hal.h"
#include "main.h"
#include "log.h"

static T_LED_MODE g_LED_mode = LED_MODE_BLINK_SLOW;

void LED_setMode(T_LED_MODE p_mode)
{
  LOG_info("Setting LED mode to %u", p_mode);

  g_LED_mode = p_mode;

  return;
}

T_LED_MODE LED_getMode(void)
{
  return g_LED_mode;
}

void LED_update(void)
{
  static uint32_t l_blinkCounter = 0;

  switch (g_LED_mode)
  {
    case LED_MODE_FORCED_OFF:
      HAL_GPIO_WritePin(BLUE_LED_GPIO_Port, BLUE_LED_Pin, GPIO_PIN_SET);
      break;

    case LED_MODE_FORCED_ON:
      HAL_GPIO_WritePin(BLUE_LED_GPIO_Port, BLUE_LED_Pin, GPIO_PIN_RESET);
      break;

    case LED_MODE_BLINK_SLOW:
      l_blinkCounter++;
      if (l_blinkCounter % 5 == 0)
      {
        HAL_GPIO_TogglePin(BLUE_LED_GPIO_Port, BLUE_LED_Pin);
      }
      else
      {
        ; /* Nothing to do */
      }
      break;

    case LED_MODE_BLINK_FAST:
      l_blinkCounter++;
      if (l_blinkCounter % 2 == 0)
      {
        HAL_GPIO_TogglePin(BLUE_LED_GPIO_Port, BLUE_LED_Pin);
      }
      else
      {
        ; /* Nothing to do */
      }
      break;

    case LED_MODE_BATTERY_LOW:
      HAL_GPIO_TogglePin(BLUE_LED_GPIO_Port, BLUE_LED_Pin);
      break;

    default:
      ; /* Nothing to do */
      break;
  }

  return;
}
