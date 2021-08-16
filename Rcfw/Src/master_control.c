#include <string.h>
#include <stdbool.h>

#include "master_control.h"

#include "stm32f1xx_hal.h"
#include "string_fifo.h"
#include "const.h"
#include "log.h"

typedef struct T_MAS_Context
{
  UART_HandleTypeDef *uartHandle;
  uint8_t             datum;
  T_SFO_data          data;
  uint32_t            dataSize;
  bool                isDataComplete;
} T_MAS_Context;

static T_MAS_Context g_MAS_context;

void MAS_init(UART_HandleTypeDef *p_uartHandle)
{
  LOG_info("Initializing master connection");

  g_MAS_context.uartHandle     = p_uartHandle;
  g_MAS_context.dataSize       = 0;
  g_MAS_context.isDataComplete = false;

  SFO_initData(&g_MAS_context.data);

  HAL_UART_Receive_IT(g_MAS_context.uartHandle, &g_MAS_context.datum, 1);

  return;
}

void MAS_receiveData(void)
{
  /* If previous command was not processed yet, drop datum, */
  /* which will lead to an error, showing a design issue... */
  if (g_MAS_context.isDataComplete == true)
  {
    ; /* Nothing to do */
  }
  else
  {
    if (g_MAS_context.datum == '\r')
    {
      g_MAS_context.isDataComplete = true;
    }
    else
    {
      g_MAS_context.data[g_MAS_context.dataSize] = g_MAS_context.datum;
      g_MAS_context.dataSize++;
    }
  }

  HAL_UART_Receive_IT(g_MAS_context.uartHandle, &g_MAS_context.datum, 1);

  return;
}

void MAS_updateFifo(T_SFO_Handle *p_fifo)
{
  if (g_MAS_context.isDataComplete == false)
  {
    ; /* Nothing to do */
  }
  else
  {
    SFO_push    (p_fifo, &g_MAS_context.data);
    SFO_initData(        &g_MAS_context.data );

    g_MAS_context.dataSize       = 0;
    g_MAS_context.isDataComplete = false;
  }

  return;
}

void MAS_sendString(char *p_string, uint32_t p_length)
{
  /* As this method is using for logging/debug, we will not deal with failure cases */
 (void)HAL_UART_Transmit(g_MAS_context.uartHandle,
              (uint8_t *)p_string,
                         p_length,
                         CST_UART_TRANSMIT_TIMEOUT_IN_MS);

  return;
}
