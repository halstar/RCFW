#include "encoder.h"

#include "log.h"

void ENC_init(T_ENC_Handle      *p_handle,
              char              *p_name,
              bool               p_invertOnUpdate,
              TIM_HandleTypeDef *p_encoderTimerHandle)
{
  LOG_info("Initializing encoder module for %s", p_name);

  p_handle->name           = p_name;
  p_handle->invertOnUpdate = p_invertOnUpdate;
  p_handle->timerHandle    = p_encoderTimerHandle;

  ENC_reset(p_handle);

  return;
}

void ENC_reset(T_ENC_Handle *p_handle)
{
  p_handle->value = 0;

  __HAL_TIM_SET_COUNTER(p_handle->timerHandle, 0);

  return;
}

void ENC_update(T_ENC_Handle *p_handle, int32_t p_value)
{
  if (p_handle->invertOnUpdate == false)
  {
    p_handle->value = p_value;
  }
  else
  {
    p_handle->value = -p_value;
  }

  return;
}

int32_t ENC_getCount(T_ENC_Handle *p_handle)
{
  int32_t l_count;

  l_count = p_handle->value;

  ENC_reset(p_handle);

  return l_count;
}

void ENC_logInfo(T_ENC_Handle *p_handle)
{
  LOG_debug("%s encoder count: %u", p_handle->name, p_handle->value);

  return;
}
