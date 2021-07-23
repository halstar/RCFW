#include <stdio.h>

#include "stm32f1xx_hal.h"
#include "log.h"

static uint32_t           LOG_level;
static RTC_HandleTypeDef *LOG_rtcHandle;

static const char *LOG_levelStrings[] =
{
  "DEBUG", "INFO", "WARN", "ERROR"
};

void LOG_init(RTC_HandleTypeDef *p_rctHandle)
{
  LOG_rtcHandle = p_rctHandle;

  return;
}

void LOG_setLevel(T_LOG_LEVEL p_level)
{
  LOG_level = p_level;

  return;
}

void LOG_log(T_LOG_LEVEL p_level, const char *p_format, ...)
{
  va_list         l_argumentsList;
  RTC_TimeTypeDef l_time;
  RTC_DateTypeDef l_date;

  if (p_level >= LOG_level)
  {
    HAL_RTC_GetTime(LOG_rtcHandle, &l_time, RTC_FORMAT_BCD);
    HAL_RTC_GetDate(LOG_rtcHandle, &l_date, RTC_FORMAT_BCD);

    va_start(l_argumentsList, p_format);

    (void)printf("%-5s - %02x:%02x:%02x - ", LOG_levelStrings[p_level], l_time.Hours, l_time.Minutes, l_time.Seconds);
    (void)vprintf(p_format, l_argumentsList);
    (void)printf("\r\n");

    va_end(l_argumentsList);
  }
  else
  {
    ; /* Nothing to do */
  }

  return;
}
