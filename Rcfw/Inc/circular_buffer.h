#ifndef __CIRCULAR_BUFFER_H
#define __CIRCULAR_BUFFER_H

#include <stdbool.h>

#include "stm32f1xx_hal.h"
#include "const.h"

typedef struct T_CBU_Context
{
  float    elements[CST_CIRCULAR_BUFFER_MAX_ELEMENTS];
  uint32_t index;
  uint32_t count;
} T_CBU_Context;

void     CBU_init      (T_CBU_Context *p_buffer                 );
void     CBU_push      (T_CBU_Context *p_buffer, float p_element);
uint32_t CBU_getCount  (T_CBU_Context *p_buffer                 );
float    CBU_getAverage(T_CBU_Context *p_buffer                 );
void     CBU_logInfo   (T_CBU_Context *p_buffer                 );

#endif /* __CIRCULAR_BUFFER_H */
