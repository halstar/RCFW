#include <stdio.h>
#include <stdbool.h>

#include "log.h"

#include "stm32f1xx_hal.h"

static bool               g_LOG_isOn;
static T_LOG_LEVEL        g_LOG_level;
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

void LOG_turnOn(void)
{
  if (g_LOG_isOn == false)
  {
    g_LOG_isOn = true;

    LOG_info("Turning LOG ON");
  }
  else
  {
    ; /* Nothing to do */
  }

  return;
}

void LOG_turnOff(void)
{
  if (g_LOG_isOn == true)
  {
    LOG_info("Turning LOG OFF");

    g_LOG_isOn = false;
  }
  else
  {
    ; /* Nothing to do */
  }

  return;
}

void LOG_setLevel(T_LOG_LEVEL p_level)
{
  g_LOG_level = p_level;

  return;
}

void LOG_increaseLevel(void)
{
  /* Display more detailed logs */
  if (g_LOG_level > LOG_LEVEL_DEBUG)
  {
    LOG_info("Increasing LOG level");

    g_LOG_level--;
  }
  else
  {
    ; /* Nothing to do */
  }

  return;
}

void LOG_decreaseLevel(void)
{
  /* Display less detailed logs */
  if (g_LOG_level < LOG_LEVEL_ERROR)
  {
    LOG_info("Decreasing LOG level");

    g_LOG_level++;
  }
  else
  {
    ; /* Nothing to do */
  }

  return;
}

void LOG_log(T_LOG_LEVEL p_level, const char *p_format, ...)
{
  va_list         l_argumentsList;
  RTC_TimeTypeDef l_time;
  RTC_DateTypeDef l_date;

  if ((g_LOG_isOn == true) && (p_level >= g_LOG_level))
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
