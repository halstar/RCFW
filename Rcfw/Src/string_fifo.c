#include <stdlib.h>
#include <string.h>

#include "string_fifo.h"

#include "log.h"

void SFO_initData(T_SFO_data *p_data)
{
  (void)memset(p_data, 0, CST_STRING_FIFO_MAX_STRING_LENGTH);

  return;
}

void SFO_init(T_SFO_Handle *p_fifo)
{
  LOG_debug("String FIFO now empty");

  p_fifo->head  = NULL;
  p_fifo->tail  = NULL;
  p_fifo->count = 0;

  return;
}

void SFO_push(T_SFO_Handle *p_fifo, T_SFO_data *p_dataIn)
{
  T_SFO_Element *l_element;

  if (p_fifo->count == CST_STRING_FIFO_MAX_ELEMENTS)
  {
    LOG_error("SFO_push() failure - String FIFO is full (%u elements)", CST_STRING_FIFO_MAX_ELEMENTS);
  }
  else
  {
    l_element = (T_SFO_Element *)malloc(sizeof(T_SFO_Element));

    if (l_element == NULL)
    {
      LOG_error("SFO_push() failure - Could not allocate new memory");
    }
    else
    {
      (void)strncpy(l_element->data, *p_dataIn, CST_STRING_FIFO_MAX_STRING_LENGTH);

      l_element->nextElement = p_fifo->head;
      p_fifo->head           = l_element;

      if (p_fifo->tail == NULL)
      {
        p_fifo->tail = p_fifo->head;
      }

      p_fifo->count++;
    }
  }

  return;
}

void SFO_pop(T_SFO_Handle *p_fifo, T_SFO_data *p_dataOut)
{
  T_SFO_Element *l_element;

  if (p_fifo->count == 0)
  {
    LOG_error("SFO_pop() failure - String FIFO is empty");
  }
  else
  {
    (void)strncpy(*p_dataOut, p_fifo->tail->data, CST_STRING_FIFO_MAX_STRING_LENGTH);

    free(p_fifo->tail);

    if (p_fifo->count > 1)
    {
      l_element = p_fifo->head;

      while (l_element->nextElement != p_fifo->tail)
      {
        l_element = l_element->nextElement;
      }

      l_element->nextElement = NULL;
      p_fifo->tail           = l_element;

      p_fifo->count--;
    }
    else
    {
      SFO_init(p_fifo);
    }
  }

  return;
}

uint32_t SFO_getCount(T_SFO_Handle *p_fifo)
{
  return p_fifo->count;
}

void SFO_logInfo(T_SFO_Handle *p_fifo)
{
  T_SFO_Element *l_element;

  if (p_fifo->count == 0)
  {
    LOG_info("String FIFO is empty");
  }
  else
  {
    LOG_info("String FIFO: %u element(s)", p_fifo->count);

    l_element = p_fifo->head;

    while (l_element != NULL)
    {
      if ((l_element == p_fifo->head) && (l_element == p_fifo->tail))
      {
        LOG_info("HEAD -> '%s' <- TAIL", l_element->data);
      }
      else if (l_element == p_fifo->head)
      {
        LOG_info("HEAD -> '%s'", l_element->data);
      }
      else if (l_element == p_fifo->tail)
      {
        LOG_info("TAIL -> '%s'", l_element->data);
      }
      else
      {
        LOG_info("        '%s'", l_element->data);
      }

      l_element = l_element->nextElement;
    }
  }

  return;
}
