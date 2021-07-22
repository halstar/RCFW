#include <stdio.h>

#include "stm32f1xx_hal.h"
#include "log.h"

static uint32_t LOG_level;

extern RTC_HandleTypeDef hrtc;

static const char *LOG_levelStrings[] =
{
  "DEBUG", "INFO", "WARN", "ERROR"
};

void LOG_setLevel(LOG_LEVEL level)
{
  LOG_level = level;

  return;
}

void LOG_log(LOG_LEVEL level, const char *file, int line, const char *format, ...)
{
  va_list         argumentsList;
  RTC_TimeTypeDef time;
  RTC_DateTypeDef date;

  if (level >= LOG_level)
  {
    HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BCD);
    HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BCD);

    va_start(argumentsList, format);

    printf("%02x:%02x:%02x %-5s %s:%d: ", time.Hours, time.Minutes, time.Seconds, LOG_levelStrings[level], file, line);
    vprintf(format, argumentsList);
    printf("\r\n");
    fflush(stdout);

    va_end(argumentsList);
  }
  else
  {
    ; /* Nothing to do */
  }

  return;
}
