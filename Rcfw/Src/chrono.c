#include "chrono.h"

#include "stm32f1xx_hal.h"
#include "log.h"

typedef struct T_CHR_Context
{
  TIM_HandleTypeDef *timerHandle;
  bool               isStarted;
  bool               wasOverflowed;
} T_CHR_Context;

static T_CHR_Context g_CHR_context;

void CHR_init(TIM_HandleTypeDef *p_usTimerHandle)
{
  LOG_info("Initializing micro-second chronometer");

  g_CHR_context.timerHandle = p_usTimerHandle;
  g_CHR_context.isStarted   = false;

  CHR_reset();

  return;
}

void CHR_start(void)
{
  HAL_StatusTypeDef l_halReturnCode;

  if (g_CHR_context.isStarted == true)
  {
    LOG_error("Trying to start chronometer twice");
  }
  else
  {
    CHR_reset();

    l_halReturnCode = HAL_TIM_Base_Start_IT(g_CHR_context.timerHandle);

    if (l_halReturnCode != HAL_OK)
    {
      LOG_error("HAL_TIM_Base_Start_IT() returned an error code: %d", l_halReturnCode);
    }
    else
    {
      g_CHR_context.isStarted = true;
    }
  }

  return;
}

void CHR_stop(void)
{
  HAL_StatusTypeDef l_halReturnCode;

  if (g_CHR_context.isStarted == false)
  {
    LOG_error("Trying to stop a chronometer that is not started");
  }
  else
  {
    l_halReturnCode = HAL_TIM_Base_Stop_IT(g_CHR_context.timerHandle);

    if (l_halReturnCode != HAL_OK)
    {
      LOG_error("HAL_TIM_Base_Stop_IT() returned an error code: %d", l_halReturnCode);
    }
    else
    {
      g_CHR_context.isStarted = false;
    }
  }

  return;
}

uint32_t CHR_getTimeUs(void)
{
  uint32_t l_currentTimeInUs;

  if (g_CHR_context.wasOverflowed == true)
  {
    LOG_warning("Chronometer overflowed");
  }
  else
  {
    ; /* Nothing to do */
  }

  l_currentTimeInUs = __HAL_TIM_GET_COUNTER(g_CHR_context.timerHandle);

  return l_currentTimeInUs;
}

void CHR_overflow(void)
{
  g_CHR_context.wasOverflowed = true;

  return;
}

void CHR_reset(void)
{
  __HAL_TIM_SET_COUNTER(g_CHR_context.timerHandle, 0);
  __HAL_TIM_CLEAR_IT   (g_CHR_context.timerHandle, TIM_IT_UPDATE);

  g_CHR_context.wasOverflowed = false;

  return;
}
