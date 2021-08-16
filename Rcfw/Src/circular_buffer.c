#include "circular_buffer.h"

#include "log.h"

void CBU_init(T_CBU_Handle *p_buffer)
{
  uint32_t l_index;

  LOG_debug("Initializing circular buffer");

  for (l_index = 0; l_index < CST_CIRCULAR_BUFFER_MAX_ELEMENTS; l_index++)
  {
    p_buffer->elements[l_index] = 0.0f;
  }

  p_buffer->count = 0;
  p_buffer->index = 0;

  return;
}

void CBU_push(T_CBU_Handle *p_buffer, float p_element)
{
  if (p_buffer->index == CST_CIRCULAR_BUFFER_MAX_ELEMENTS - 1)
  {
    p_buffer->index = 0;
  }
  else
  {
    p_buffer->index++;
  }

  p_buffer->elements[p_buffer->index] = p_element;

  if (p_buffer->count < CST_CIRCULAR_BUFFER_MAX_ELEMENTS)
  {
    p_buffer->count++;
  }
  else
  {
    ; /* Nothing to do */
  }

  return;
}

uint32_t CBU_getCount(T_CBU_Handle *p_buffer)
{
  return p_buffer->count;
}

float CBU_getAverage(T_CBU_Handle *p_buffer)
{
  uint32_t l_index;
  uint32_t l_count;
  float    l_average;

  l_index   = p_buffer->index;
  l_count   = 0;
  l_average = 0.0f;

  while (l_count < p_buffer->count)
  {
    l_average += p_buffer->elements[l_index];

    if (l_index == CST_CIRCULAR_BUFFER_MAX_ELEMENTS - 1)
    {
      l_index = 0;
    }
    else
    {
      l_index++;
    }

    l_count++;
  }

  l_average /= (float)p_buffer->count;

  return l_average;
}

void CBU_logInfo(T_CBU_Handle *p_buffer)
{
  uint32_t l_index;
  uint32_t l_count;

  if (p_buffer->count == 0)
  {
    LOG_info("Circular buffer is empty");
  }
  else
  {
    LOG_info("Circular buffer: %u element(s)", p_buffer->count);

    l_index = p_buffer->index;
    l_count = 0;

    while (l_count < p_buffer->count)
    {
      LOG_info("@%u: %f", l_index, p_buffer->elements[l_index]);

      if (l_index == CST_CIRCULAR_BUFFER_MAX_ELEMENTS - 1)
      {
        l_index = 0;
      }
      else
      {
        l_index++;
      }

      l_count++;
    }
  }

  return;
}
