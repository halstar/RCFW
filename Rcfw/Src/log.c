#include "log.h"

#include "stdio.h"

#include <time.h>

uint32_t LOG_level;

static const char *levelStrings[] = {
  "DEBUG", "INFO", "WARN", "ERROR"
};

void LOG_setLevel(int level) {
  LOG_level = level;

  return;
}

void LOG_log(int level, const char *file, int line, const char *format, ...) {

  va_list argumentsList;
  char buffer[16];

  time_t currentTime = time(NULL);
  struct tm *localTime = localtime(&currentTime);

  if (level >= LOG_level) {
    va_start(argumentsList, format);

    buffer[strftime(buffer, sizeof(buffer), "%H:%M:%S", localTime)] = '\0';
    fprintf(stdout, "%s %-5s %s:%d: ", buffer, levelStrings[level], file, line);
    vfprintf(stdout, format, argumentsList);
    fprintf(stdout, "\n");
    fflush(stdout);

    va_end(argumentsList);
  }

  return;
}
