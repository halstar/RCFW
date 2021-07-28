#ifndef __LOG_H
#define __LOG_H

#include <stdarg.h>

#include "stm32f1xx_hal.h"

typedef enum
{
  LOG_LEVEL_DEBUG = 0,
  LOG_LEVEL_INFO,
  LOG_LEVEL_WARN,
  LOG_LEVEL_ERROR
} T_LOG_LEVEL;

void LOG_init         (RTC_HandleTypeDef *p_rctHandle                );
void LOG_turnOn       (void                                          );
void LOG_turnOff      (void                                          );
void LOG_setLevel     (T_LOG_LEVEL p_level                           );
void LOG_increaseLevel(void                                          );
void LOG_decreaseLevel(void                                          );
void LOG_log          (T_LOG_LEVEL p_level, const char *p_format, ...);

#define LOG_debug(...)   LOG_log(LOG_LEVEL_DEBUG, __VA_ARGS__)
#define LOG_info(...)    LOG_log(LOG_LEVEL_INFO , __VA_ARGS__)
#define LOG_warning(...) LOG_log(LOG_LEVEL_WARN , __VA_ARGS__)
#define LOG_error(...)   LOG_log(LOG_LEVEL_ERROR, __VA_ARGS__)

#endif /* __LOG_H */
