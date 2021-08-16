#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "utils.h"
#include "const.h"
#include "log.h"

#include "stm32f1xx_hal.h"

typedef struct T_LOG_Context
{
  bool               isOn;
  T_LOG_LEVEL        level;
  RTC_HandleTypeDef *rtcHandle;
} T_LOG_Context;

static T_LOG_Context g_LOG_context;

static const char *g_LOG_levelStrings[4] =
{
  "DEBUG", "INFO", "WARNING", "ERROR"
};

void LOG_init(RTC_HandleTypeDef *p_rctHandle, bool p_isLogOn)
{
  g_LOG_context.isOn      = p_isLogOn;
  g_LOG_context.rtcHandle = p_rctHandle;

  LOG_info("LOG initialized");

  return;
}

void LOG_toggleOnOff(void)
{
  if (g_LOG_context.isOn == true)
  {
    LOG_info("Turning LOG OFF");

    g_LOG_context.isOn = false;
  }
  else
  {
    g_LOG_context.isOn = true;

    LOG_info("Turning LOG ON");
  }

  return;
}

void LOG_setLevel(T_LOG_LEVEL p_level)
{
  g_LOG_context.level = p_level;

  return;
}

void LOG_increaseLevel(void)
{
  /* Display more detailed logs */
  if (g_LOG_context.level > LOG_LEVEL_DEBUG)
  {
    LOG_info("Increasing LOG level");

    g_LOG_context.level--;
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
  if (g_LOG_context.level < LOG_LEVEL_ERROR)
  {
    LOG_info("Decreasing LOG level");

    g_LOG_context.level++;
  }
  else
  {
    ; /* Nothing to do */
  }

  return;
}

void LOG_log(T_LOG_LEVEL p_level, const char *p_format, ...)
{
  char              l_buffer[CST_CONSOLE_TX_MAX_STRING_LENGTH];
  HAL_StatusTypeDef l_halReturnCode;
  va_list           l_argumentsList;
  RTC_TimeTypeDef   l_time;
  RTC_DateTypeDef   l_date;

  if ((g_LOG_context.isOn == true) && (p_level >= g_LOG_context.level))
  {
    l_halReturnCode = HAL_RTC_GetTime(g_LOG_context.rtcHandle, &l_time, RTC_FORMAT_BCD);

    if (l_halReturnCode != HAL_OK)
    {
      /* As this method is for logging/debug, just reset time in case of failure */
      UTI_resetRtcTime(&l_time);
    }
    else
    {
      ; /* Nothing to do */
    }

    l_halReturnCode = HAL_RTC_GetDate(g_LOG_context.rtcHandle, &l_date, RTC_FORMAT_BCD);

    if (l_halReturnCode != HAL_OK)
    {
      /* As this method is for logging/debug, just reset date in case of failure */
      UTI_resetRtcDate(&l_date);
    }
    else
    {
      ; /* Nothing to do */
    }

    va_start(l_argumentsList, p_format);

    (void)sprintf  ( l_buffer, "%-7s - %02x:%02x:%02x - ", g_LOG_levelStrings[p_level], l_time.Hours, l_time.Minutes, l_time.Seconds);
    /* 21 is the length of string "%-7s - %02x:%02x:%02x - ". 2 is the length of string "\r\n". */
    /* So we start and write format right after the former and we keep space for the latter.    */
    (void)vsnprintf(&l_buffer[21], CST_CONSOLE_TX_MAX_STRING_LENGTH - 21 - 2, p_format, l_argumentsList);
    (void)strcat   ( l_buffer    , "\r\n");

    /* -1 is there to consider the end/null character, which strnlen() does not count */
    if (strnlen(l_buffer, CST_CONSOLE_TX_MAX_STRING_LENGTH) == CST_CONSOLE_TX_MAX_STRING_LENGTH - 1)
    {
      /* At -1, we got the end/null character. At -2 & 3, we got "\r\n". */
      l_buffer[CST_CONSOLE_TX_MAX_STRING_LENGTH - 4] = '.';
      l_buffer[CST_CONSOLE_TX_MAX_STRING_LENGTH - 5] = '.';
      l_buffer[CST_CONSOLE_TX_MAX_STRING_LENGTH - 6] = '.';
    }
    else
    {
      ; /* Nothing to do */
    }

    (void)printf("%s", l_buffer);

    va_end(l_argumentsList);
  }
  else
  {
    ; /* Nothing to do */
  }

  return;
}
