#include "utils.h"

#include "stm32f1xx_hal.h"
#include "log.h"

static TIM_HandleTypeDef *g_UTI_delayTimerHandle;

void UTI_init(TIM_HandleTypeDef *p_delayTimerHandle)
{
  LOG_info("Initializing utilities");

  g_UTI_delayTimerHandle = p_delayTimerHandle;

  return;
}

void UTI_delayUs(uint16_t p_delay)
{
  /* Reset the micro-seconds counter */
  __HAL_TIM_SET_COUNTER(g_UTI_delayTimerHandle, 0);

  /* Wait for the counter to reach the input micro-seconds number */
  while (__HAL_TIM_GET_COUNTER(g_UTI_delayTimerHandle) < p_delay)
  {
    ; /* Nothing to do */
  }

  return;
}

void UTI_resetRtcDate(RTC_DateTypeDef *p_date)
{
  p_date->Date    = 0;
  p_date->Month   = 0;
  p_date->WeekDay = 0;
  p_date->Year    = 0;

  return;
}


void UTI_resetRtcTime(RTC_TimeTypeDef *p_time)
{
  p_time->Hours   = 0;
  p_time->Minutes = 0;
  p_time->Seconds = 0;

  return;
}

uint32_t UTI_turnRtcTimeToSeconds(RTC_TimeTypeDef *p_time)
{
  return p_time->Hours * 3600 + p_time->Minutes * 60 + p_time->Seconds;
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
    l_returnValue = p_value;
  }

  return l_returnValue;
}

float UTI_clampFloatValue(float p_value, float p_minValue, float p_maxValue, bool p_clampToNearest, float p_clampValue)
{
  float l_returnValue;

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
    l_returnValue = p_value;
  }

  return l_returnValue;
}

int32_t UTI_normalizeIntValue(int32_t p_value, int32_t p_inMinValue, int32_t p_inMaxValue, int32_t p_outMinValue, int32_t p_outMaxValue, bool p_isInversionNeeded)
{
  float l_returnValue;
  float l_ratio;

  l_ratio       = ((float)p_outMaxValue - (float)p_outMinValue) / ((float)p_inMaxValue - (float)p_inMinValue);
  l_returnValue =  (float)(p_value - p_inMinValue) * l_ratio + p_outMinValue;

  if (p_isInversionNeeded == true)
  {
    l_returnValue *= -1.0f;
  }
  else
  {
    ; /* Nothing to do */
  }

  return (int32_t)l_returnValue;
}
