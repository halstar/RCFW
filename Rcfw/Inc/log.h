#ifndef __LOG_H
#define __LOG_H

#include <stdbool.h>
#include <stdarg.h>

typedef enum
{
  LOG_LEVEL_DEBUG = 0,
  LOG_LEVEL_INFO,
  LOG_LEVEL_WARN,
  LOG_LEVEL_ERROR
} T_LOG_LEVEL;

void LOG_init         (bool        p_isLogOn                         );
void LOG_toggleOnOff  (void                                          );
void LOG_setLevel     (T_LOG_LEVEL p_level                           );
void LOG_increaseLevel(void                                          );
void LOG_decreaseLevel(void                                          );
void LOG_log          (T_LOG_LEVEL p_level, const char *p_format, ...);

#ifdef DEBUG

  #define LOG_debug(...)   LOG_log(LOG_LEVEL_DEBUG, __VA_ARGS__)
  #define LOG_info(...)    LOG_log(LOG_LEVEL_INFO , __VA_ARGS__)
  #define LOG_warning(...) LOG_log(LOG_LEVEL_WARN , __VA_ARGS__)
  #define LOG_error(...)   LOG_log(LOG_LEVEL_ERROR, __VA_ARGS__)

#else /* RELEASE */

  #define LOG_debug(...)
  #define LOG_info(...)
  #define LOG_warning(...)
  #define LOG_error(...)

#endif

#endif /* __LOG_H */
