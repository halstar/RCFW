#include "utils.h"

#include "stm32f1xx_hal.h"
#include "log.h"

static TIM_HandleTypeDef *UTI_delayTimerHandle;

void UTI_init(TIM_HandleTypeDef *p_delayTimerHandle)
{
  LOG_info("Initializing utilities");

  UTI_delayTimerHandle = p_delayTimerHandle;

  return;
}

void UTI_delayUs(uint16_t p_delay)
{
  /* Reset the micro-seconds counter */
  __HAL_TIM_SET_COUNTER(UTI_delayTimerHandle,0);

  /* Wait for the counter to reach the input micro-seconds number */
  while (__HAL_TIM_GET_COUNTER(UTI_delayTimerHandle) < p_delay)
  {
    ; /* Nothing to do */
  }

  return;
}

int32_t UTI_clampIntValue(int32_t p_value, int32_t p_minValue, int32_t p_maxValue, bool p_clampToNearest, int32_t p_clampValue)
{
  int32_t l_returnValue;

  if (p_value < p_minValue)
  {
    if (p_clampToNearest == true)
    {
      l_returnValue = p_minValue;
    }
    else
    {
      l_returnValue = p_clampValue;
    }
  }
  else if (p_value > p_maxValue)
  {
    if (p_clampToNearest == true)
    {
      l_returnValue = p_maxValue;
    }
    else
    {
      l_returnValue = p_clampValue;
    }
  }
  else
  {
    ; /* Nothing to do */
  }

  return l_returnValue;
}
