#ifndef __STRING_FIFO_H
#define __STRING_FIFO_H

#include <stdbool.h>

#include "stm32f1xx_hal.h"

#define SFO_MAX_ELEMENTS    10
#define SFO_MAX_STRING_SIZE 16

typedef char T_SFO_data[SFO_MAX_STRING_SIZE];

typedef struct T_SFO_Element T_SFO_Element;

typedef struct T_SFO_Element
{
  T_SFO_data     data;
  T_SFO_Element *nextElement;
} T_SFO_Element;

typedef struct
{
  T_SFO_Element *head;
  T_SFO_Element *tail;
  uint32_t       count;
} T_SFO_Context;

void     SFO_dataInit(T_SFO_data    *p_data                       );
void     SFO_init    (T_SFO_Context *p_fifo                       );
void     SFO_push    (T_SFO_Context *p_fifo, T_SFO_data *p_dataIn );
void     SFO_pop     (T_SFO_Context *p_fifo, T_SFO_data *p_dataOut);
uint32_t SFO_getCount(T_SFO_Context *p_fifo                       );
void     SFO_logInfo (T_SFO_Context *p_fifo                       );

#endif /* __STRING_FIFO_H */
