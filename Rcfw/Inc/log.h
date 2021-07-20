#ifndef __LOG_H
#define __LOG_H

#include <stdarg.h>

enum {LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR};

void LOG_setLevel(int level);
void LOG_log(int level, const char *file, int line, const char *fmt, ...);

#define LOG_debug(...) LOG_log(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_info(...)  LOG_log(LOG_INFO,  __FILE__, __LINE__, __VA_ARGS__)
#define LOG_warn(...)  LOG_log(LOG_WARN,  __FILE__, __LINE__, __VA_ARGS__)
#define LOG_error(...) LOG_log(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)

#endif /* __LOG_H */
