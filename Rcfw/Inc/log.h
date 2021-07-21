#ifndef __LOG_H
#define __LOG_H

#include <stdarg.h>

typedef enum
{
  LOG_LEVEL_DEBUG = 0,
  LOG_LEVEL_INFO,
  LOG_LEVEL_WARN,
  LOG_LEVEL_ERROR
} LOG_LEVEL;

void LOG_setLevel(LOG_LEVEL level);
void LOG_log     (LOG_LEVEL level, const char *file, int line, const char *fmt, ...);

#define LOG_debug(...) LOG_log(LOG_LEVEL_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_info(...)  LOG_log(LOG_LEVEL_INFO,  __FILE__, __LINE__, __VA_ARGS__)
#define LOG_warn(...)  LOG_log(LOG_LEVEL_WARN,  __FILE__, __LINE__, __VA_ARGS__)
#define LOG_error(...) LOG_log(LOG_LEVEL_ERROR, __FILE__, __LINE__, __VA_ARGS__)

#endif /* __LOG_H */
