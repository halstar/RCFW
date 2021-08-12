#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "console.h"

#include "stm32f1xx_hal.h"
#include "string_fifo.h"
#include "const.h"
#include "log.h"

static UART_HandleTypeDef *g_CON_uartHandle;
static uint8_t             g_CON_datum;
static T_SFO_data          g_CON_data;
static uint32_t            g_CON_dataSize;
static bool                g_CON_isDataComplete;

void CON_init(UART_HandleTypeDef *p_uartHandle)
{
  LOG_info("Initializing console");

  g_CON_uartHandle     = p_uartHandle;
  g_CON_dataSize       = 0;
  g_CON_isDataComplete = false;

  SFO_initData(&g_CON_data);

  HAL_UART_Receive_IT(g_CON_uartHandle, &g_CON_datum, 1);

  return;
}

void CON_receiveData(void)
{
  /* If previous command was not processed yet, drop datum, */
  /* which will lead to an error, showing a design issue... */
  if (g_CON_isDataComplete == true)
  {
    ; /* Nothing to do */
  }
  else
  {
    if (g_CON_datum == '\r')
    {
      g_CON_isDataComplete = true;
    }
    else
    {
      g_CON_data[g_CON_dataSize] = g_CON_datum;
      g_CON_dataSize++;
    }
  }

  HAL_UART_Receive_IT(g_CON_uartHandle, &g_CON_datum, 1);

  return;
}

void CON_updateFifo(T_SFO_Context *p_fifo)
{
  if (g_CON_isDataComplete == false)
  {
    ; /* Nothing to do */
  }
  else
  {
    SFO_push    (p_fifo, &g_CON_data);
    SFO_initData(&g_CON_data);

    g_CON_dataSize       = 0;
    g_CON_isDataComplete = false;
  }

  return;
}

void CON_sendString(char *p_string, uint32_t p_length)
{
  /* As this method is using for logging/debug, we will not deal with failure cases */
  (void)HAL_UART_Transmit_IT(g_CON_uartHandle,
                  (uint8_t *)p_string,
                             p_length);

  return;
}
