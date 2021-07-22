#include "battery_check.h"

#include "log.h"

#define BATTERY_CHECK_POLLING_PERIOD_IN_S 10

static ADC_HandleTypeDef *BATTERY_CHECK_adcHandle;
static RTC_HandleTypeDef *BATTERY_CHECK_rtcHandle;
static RTC_TimeTypeDef    BATTERY_CHECK_lastReadTime;

void BATTERY_CHECK_init(ADC_HandleTypeDef *p_adcHandle, RTC_HandleTypeDef *p_rtcHandle)
{
  RTC_TimeTypeDef l_time;
  RTC_DateTypeDef l_date;

  LOG_info("Initializing battery check");

  BATTERY_CHECK_adcHandle = p_adcHandle;
  BATTERY_CHECK_rtcHandle = p_rtcHandle;

  HAL_RTC_GetTime(BATTERY_CHECK_rtcHandle, &l_time, RTC_FORMAT_BCD);
  HAL_RTC_GetDate(BATTERY_CHECK_rtcHandle, &l_date, RTC_FORMAT_BCD);

  BATTERY_CHECK_lastReadTime = l_time;

  return;
}

void BATTERY_CHECK_update(void)
{
  RTC_TimeTypeDef l_time;
  RTC_DateTypeDef l_date;
  uint32_t        l_adcRawData;

  HAL_RTC_GetTime(BATTERY_CHECK_rtcHandle, &l_time, RTC_FORMAT_BCD);
  HAL_RTC_GetDate(BATTERY_CHECK_rtcHandle, &l_date, RTC_FORMAT_BCD);

  if ((l_time.Hours * 3600 + l_time.Minutes * 60 + l_time.Seconds) -
      (BATTERY_CHECK_lastReadTime.Hours * 3600 + BATTERY_CHECK_lastReadTime.Minutes * 60 + BATTERY_CHECK_lastReadTime.Seconds) >= BATTERY_CHECK_POLLING_PERIOD_IN_S)
  {
    HAL_ADC_Start                (BATTERY_CHECK_adcHandle);
    HAL_ADC_PollForConversion    (BATTERY_CHECK_adcHandle, HAL_MAX_DELAY);
    l_adcRawData = HAL_ADC_GetValue(BATTERY_CHECK_adcHandle);

    LOG_debug("Battery level: %u", l_adcRawData);

    BATTERY_CHECK_lastReadTime = l_time;
  }
  else
  {
    ; /* Nothing to do */
  }

  return;
}
