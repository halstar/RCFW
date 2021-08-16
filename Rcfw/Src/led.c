#include "led.h"

#include "stm32f1xx_hal.h"
#include "main.h"
#include "log.h"

typedef struct T_LED_Context
{
  T_LED_MODE mode;
} T_LED_Context;

static T_LED_Context g_LED_context;

void LED_setMode(T_LED_MODE p_mode)
{
  if (p_mode != g_LED_context.mode)
  {
    switch (p_mode)
    {
      case LED_MODE_FORCED_OFF:
        LOG_info("Turning LED OFF");
        break;

      case LED_MODE_FORCED_ON:
        LOG_info("Turning LED ON");
        break;

      case LED_MODE_BLINK_SLOW:
        LOG_info("Setting LED to blink slow");
        break;

      case LED_MODE_BLINK_MEDIUM:
        LOG_info("Setting LED to blink medium");
        break;

      case LED_MODE_BLINK_FAST:
        LOG_info("Setting LED to blink fast");
        break;

      default:
        ; /* Nothing to do */
        break;
    }

    g_LED_context.mode = p_mode;
  }
  else
  {
    ; /* Nothong to do */
  }

  return;
}

T_LED_MODE LED_getMode(void)
{
  return g_LED_context.mode;
}

void LED_update(void)
{
  static uint32_t l_blinkCounter = 0;

  switch (g_LED_context.mode)
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

    case LED_MODE_BLINK_MEDIUM:
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

    case LED_MODE_BLINK_FAST:
      HAL_GPIO_TogglePin(BLUE_LED_GPIO_Port, BLUE_LED_Pin);
      break;

    default:
      ; /* Nothing to do */
      break;
  }

  return;
}
