#include "utils.h"

#include "stm32f1xx_hal.h"
#include "log.h"

typedef struct T_UTI_Context
{
  TIM_HandleTypeDef *usDelayHandle;
  RTC_HandleTypeDef *sTimerHandle ;
} T_UTI_Context;

static T_UTI_Context g_UTI_context;

static void     UTI_resetRtcTime        (RTC_TimeTypeDef *p_time);
static uint32_t UTI_turnRtcTimeToSeconds(RTC_TimeTypeDef *p_time);

void UTI_init(TIM_HandleTypeDef *p_usDelayHandle,
              RTC_HandleTypeDef *p_sTimerHandle)
{
  HAL_StatusTypeDef l_halReturnCode;

  LOG_info("Initializing utilities");

  g_UTI_context.usDelayHandle = p_usDelayHandle;
  g_UTI_context.sTimerHandle  = p_sTimerHandle ;

  l_halReturnCode = HAL_TIM_Base_Start(p_usDelayHandle);

  if (l_halReturnCode != HAL_OK)
  {
    LOG_error("HAL_TIM_Base_Start() returned an error code: %d", l_halReturnCode);
  }
  else
  {
    LOG_info("Micro-second delay timer started");
  }

  return;
}

void UTI_delayUs(uint32_t p_delay)
{
  if (p_delay > UINT32_MAX)
  {
    LOG_error("Input delay, %u, is greater than maximum allowed value: %u", p_delay, UINT32_MAX);
  }
  else
  {
    /* Reset the micro-seconds counter */
    __HAL_TIM_SET_COUNTER(g_UTI_context.usDelayHandle, 0);

    /* Wait for the counter to reach the input micro-seconds number */
    while (__HAL_TIM_GET_COUNTER(g_UTI_context.usDelayHandle) < p_delay)
    {
      ; /* Nothing to do */
    }
  }

  return;
}

void UTI_delayMs(uint32_t p_delay)
{
  HAL_Delay(p_delay);

  return;
}

uint32_t UTI_getTimeMs(void)
{
  return HAL_GetTick();
}

uint32_t UTI_getTimeS(void)
{
  RTC_TimeTypeDef l_rtcTime;
  uint32_t        l_currentTimeInS;

  UTI_getTimeRtc(&l_rtcTime);

  l_currentTimeInS = UTI_turnRtcTimeToSeconds(&l_rtcTime);

  return l_currentTimeInS;
}

void UTI_getTimeRtc(RTC_TimeTypeDef *p_time)
{
  HAL_StatusTypeDef l_halReturnCode;
  RTC_TimeTypeDef   l_rtcTime;
  RTC_DateTypeDef   l_rtcDate;

  l_halReturnCode = HAL_RTC_GetTime(g_UTI_context.sTimerHandle, &l_rtcTime, RTC_FORMAT_BCD);

  if (l_halReturnCode != HAL_OK)
  {
    /* As this method can be called by logging/debug, just reset time in case of failure */
    UTI_resetRtcTime(&l_rtcTime);
  }
  else
  {
    ; /* Nothing to to */
  }

  l_halReturnCode = HAL_RTC_GetDate(g_UTI_context.sTimerHandle, &l_rtcDate, RTC_FORMAT_BCD);

  if (l_halReturnCode != HAL_OK)
  {
    /* As this method can be called by logging/debug, just reset time in case of failure */
    UTI_resetRtcTime(&l_rtcTime);
  }
  else
  {
    ; /* Nothing to to */
  }

  *p_time = l_rtcTime;

  return;
}

int32_t UTI_clampIntValue(int32_t p_value, int32_t p_minValue, int32_t p_maxValue, bool p_clampToNearest, int32_t p_clampValue)
{
  return (int32_t)UTI_clampFloatValue(p_value, p_minValue, p_maxValue, p_clampToNearest, p_clampValue);
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
  return (int32_t)UTI_normalizeFloatValue(p_value, p_inMinValue, p_inMaxValue, p_outMinValue, p_outMaxValue, p_isInversionNeeded);
}

float UTI_normalizeFloatValue(float p_value, float p_inMinValue, float p_inMaxValue, float p_outMinValue, float p_outMaxValue, bool p_isInversionNeeded)
{
  float l_returnValue;
  float l_ratio;

  l_ratio       = (p_outMaxValue - p_outMinValue) / (p_inMaxValue - p_inMinValue);
  l_returnValue =  (p_value - p_inMinValue) * l_ratio + p_outMinValue;

  if (p_isInversionNeeded == true)
  {
    l_returnValue *= -1.0f;
  }
  else
  {
    ; /* Nothing to do */
  }

  return l_returnValue;
}

static void UTI_resetRtcTime(RTC_TimeTypeDef *p_time)
{
  p_time->Hours   = 0;
  p_time->Minutes = 0;
  p_time->Seconds = 0;

  return;
}

static uint32_t UTI_turnRtcTimeToSeconds(RTC_TimeTypeDef *p_time)
{
  return p_time->Hours * 3600 + p_time->Minutes * 60 + p_time->Seconds;
}
