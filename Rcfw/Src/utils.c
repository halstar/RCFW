#include "utils.h"

#include "stm32f1xx_hal.h"

static TIM_HandleTypeDef *UTILS_delayTimerHandle;

void UTILS_delayUsInit(TIM_HandleTypeDef *p_htim)
{
  UTILS_delayTimerHandle = p_htim;

  return;
}

void UTILS_delayUs(uint16_t p_delay)
{
  /* Reset the micro-seconds counter */
  __HAL_TIM_SET_COUNTER(UTILS_delayTimerHandle,0);

  /* Wait for the counter to reach the input micro-seconds number */
  while (__HAL_TIM_GET_COUNTER(UTILS_delayTimerHandle) < p_delay)
  {
    ; /* Nothing to do */
  }

  return;
}

void UTILS_clampUIntValue(uint32_t *p_value, uint32_t p_minValue, uint32_t p_maxValue, bool p_clampToNearest, uint32_t p_clampValue)
{
  if (*p_value < p_minValue)
  {
    if (p_clampToNearest == true)
    {
      *p_value = p_minValue;
    }
    else
    {
      *p_value = p_clampValue;
    }
  }
  else if (*p_value > p_maxValue)
  {
    if (p_clampToNearest == true)
    {
      *p_value = p_maxValue;
    }
    else
    {
      *p_value = p_clampValue;
    }
  }
  else
  {
    ; /* Nothing to do */
  }

  return;
}
