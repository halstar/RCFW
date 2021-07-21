#include <stdio.h>
#include <time.h>

#include "log.h"

static uint32_t LOG_level;

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
  va_list argumentsList;
  char buffer[16];

  time_t currentTime = time(NULL);
  struct tm *localTime = localtime(&currentTime);

  if (level >= LOG_level)
  {
    va_start(argumentsList, format);

    buffer[strftime(buffer, sizeof(buffer), "%H:%M:%S", localTime)] = '\0';
    printf("%s %-5s %s:%d: ", buffer, LOG_levelStrings[level], file, line);
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
