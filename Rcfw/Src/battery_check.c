#include "battery_check.h"

#include "utils.h"
#include "log.h"

#define BAT_POLLING_PERIOD_IN_S 10

static ADC_HandleTypeDef *g_BAT_adcHandle;
static RTC_HandleTypeDef *g_BAT_rtcHandle;
static RTC_TimeTypeDef    g_BAT_lastReadTime;

void BAT_init(ADC_HandleTypeDef *p_adcHandle, RTC_HandleTypeDef *p_rtcHandle)
{
  HAL_StatusTypeDef l_halReturnCode;
  RTC_TimeTypeDef   l_time;
  RTC_DateTypeDef   l_date;

  LOG_info("Initializing battery check");

  g_BAT_adcHandle = p_adcHandle;
  g_BAT_rtcHandle = p_rtcHandle;

  l_halReturnCode = HAL_RTC_GetTime(g_BAT_rtcHandle, &l_time, RTC_FORMAT_BCD);

  if (l_halReturnCode != HAL_OK)
  {
    LOG_error("HAL_RTC_GetTime() returned an error code: %d", l_halReturnCode);
  }
  else
  {
    ; /* Nothing to to */
  }

  l_halReturnCode = HAL_RTC_GetDate(g_BAT_rtcHandle, &l_date, RTC_FORMAT_BCD);

  if (l_halReturnCode != HAL_OK)
  {
    LOG_error("HAL_RTC_GetDate() returned an error code: %d", l_halReturnCode);
  }
  else
  {
    ; /* Nothing to to */
  }

  g_BAT_lastReadTime = l_time;

  return;
}

void BAT_update(uint32_t *p_voltageInMv)
{
  HAL_StatusTypeDef l_halReturnCode;
  RTC_TimeTypeDef   l_time;
  RTC_DateTypeDef   l_date;
  uint32_t          l_adcRawData;
  float             l_voltageInV;

  l_halReturnCode = HAL_RTC_GetTime(g_BAT_rtcHandle, &l_time, RTC_FORMAT_BCD);

  if (l_halReturnCode != HAL_OK)
  {
    LOG_error("HAL_RTC_GetTime() returned an error code: %d", l_halReturnCode);
  }
  else
  {
    ; /* Nothing to to */
  }

  l_halReturnCode = HAL_RTC_GetDate(g_BAT_rtcHandle, &l_date, RTC_FORMAT_BCD);

  if (l_halReturnCode != HAL_OK)
  {
    LOG_error("HAL_RTC_GetDate() returned an error code: %d", l_halReturnCode);
  }
  else
  {
    ; /* Nothing to to */
  }

  if (UTI_turnRtcTimeToSeconds(&l_time) - UTI_turnRtcTimeToSeconds(&g_BAT_lastReadTime) >= BAT_POLLING_PERIOD_IN_S)
  {
    g_BAT_lastReadTime = l_time;

    l_halReturnCode = HAL_ADC_Start(g_BAT_adcHandle);

    if (l_halReturnCode != HAL_OK)
    {
      LOG_error("HAL_ADC_Start() returned an error code: %d", l_halReturnCode);
    }
    else
    {
      ; /* Nothing to to */
    }

    l_halReturnCode = HAL_ADC_PollForConversion(g_BAT_adcHandle, HAL_MAX_DELAY);

    if (l_halReturnCode != HAL_OK)
    {
      LOG_error("HAL_ADC_PollForConversion() returned an error code: %d", l_halReturnCode);
    }
    else
    {
      ; /* Nothing to to */
    }

    l_adcRawData = HAL_ADC_GetValue(g_BAT_adcHandle);

    /* Apply conversion based on STM32 reference voltage & resolution */
    l_voltageInV  = (l_adcRawData * 3.30f ) / 4096.0f;

    /* Consider voltage divider used between the battery & the ADC input */
    l_voltageInV *= 11.0f;

    *p_voltageInMv = (uint32_t)(l_voltageInV * 1000.0f);

    LOG_debug("Battery level: %u mV", *p_voltageInMv);
  }
  else
  {
    ; /* Nothing to do */
  }

  return;
}
