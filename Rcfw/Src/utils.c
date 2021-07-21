#include "utils.h"

#include "stm32f1xx_hal.h"

static TIM_HandleTypeDef *UTILS_delayTimerHandle;

void UTILS_delayUsInit(TIM_HandleTypeDef *htim)
{
  UTILS_delayTimerHandle = htim;

  return;
}

void UTILS_delayUs(uint16_t delay)
{
  /* Reset the micro-seconds counter */
  __HAL_TIM_SET_COUNTER(UTILS_delayTimerHandle,0);

  /* Wait for the counter to reach the input micro-seconds number */
  while (__HAL_TIM_GET_COUNTER(UTILS_delayTimerHandle) < delay)
  {
    ; /* Nothing to do */
  }

  return;
}
