#include <stdio.h>

#include "log.h"

#include "stm32f1xx_hal.h"

static uint32_t           g_LOG_level;
static RTC_HandleTypeDef *g_LOG_rtcHandle;
static const char        *g_LOG_levelStrings[] =
{
  "DEBUG", "INFO", "WARNING", "ERROR"
};

void LOG_init(RTC_HandleTypeDef *p_rctHandle)
{
  g_LOG_rtcHandle = p_rctHandle;

  return;
}

void LOG_setLevel(T_LOG_LEVEL p_level)
{
  g_LOG_level = p_level;

  return;
}

void LOG_log(T_LOG_LEVEL p_level, const char *p_format, ...)
{
  va_list         l_argumentsList;
  RTC_TimeTypeDef l_time;
  RTC_DateTypeDef l_date;

  if (p_level >= g_LOG_level)
  {
    HAL_RTC_GetTime(g_LOG_rtcHandle, &l_time, RTC_FORMAT_BCD);
    HAL_RTC_GetDate(g_LOG_rtcHandle, &l_date, RTC_FORMAT_BCD);

    va_start(l_argumentsList, p_format);

    (void)printf("%-7s - %02x:%02x:%02x - ", g_LOG_levelStrings[p_level], l_time.Hours, l_time.Minutes, l_time.Seconds);
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
