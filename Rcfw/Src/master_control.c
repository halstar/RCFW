#include <string.h>
#include <stdbool.h>

#include "master_control.h"

#include "stm32f1xx_hal.h"
#include "string_fifo.h"
#include "const.h"
#include "log.h"

static UART_HandleTypeDef *g_MAS_uartHandle;
static uint8_t             g_MAS_datum;
static T_SFO_data          g_MAS_data;
static uint32_t            g_MAS_dataSize;
static bool                g_MAS_isDataComplete;

void MAS_init(UART_HandleTypeDef *p_uartHandle)
{
  LOG_info("Initializing master connection");

  g_MAS_uartHandle     = p_uartHandle;
  g_MAS_dataSize       = 0;
  g_MAS_isDataComplete = false;

  SFO_initData(&g_MAS_data);

  HAL_UART_Receive_IT(g_MAS_uartHandle, &g_MAS_datum, 1);

  return;
}

void MAS_receiveData(void)
{
  /* If previous command was not processed yet, drop datum, */
  /* which will lead to an error, showing a design issue... */
  if (g_MAS_isDataComplete == true)
  {
    ; /* Nothing to do */
  }
  else
  {
    if (g_MAS_datum == '\r')
    {
      g_MAS_isDataComplete = true;
    }
    else
    {
      g_MAS_data[g_MAS_dataSize] = g_MAS_datum;
      g_MAS_dataSize++;
    }
  }

  HAL_UART_Receive_IT(g_MAS_uartHandle, &g_MAS_datum, 1);

  return;
}

void MAS_updateFifo(T_SFO_Context *p_fifo)
{
  if (g_MAS_isDataComplete == false)
  {
    ; /* Nothing to do */
  }
  else
  {
    SFO_push    (p_fifo, &g_MAS_data);
    SFO_initData(&g_MAS_data);

    g_MAS_dataSize       = 0;
    g_MAS_isDataComplete = false;
  }

  return;
}

void MAS_sendString(char *p_string)
{
  HAL_StatusTypeDef l_halReturnCode;

  l_halReturnCode = HAL_UART_Transmit(g_MAS_uartHandle,
                           (uint8_t *)p_string,
                              strnlen(p_string, CST_MASTER_TX_MAX_STRING_LENGTH),
                                      CST_UART_TRANSMIT_TIMEOUT_IN_MS);

  if (l_halReturnCode == HAL_OK)
  {
    ; /* Nothing to do */
  }
  else
  {
    LOG_error("HAL_UART_Transmit() returned an error code: %d", l_halReturnCode);
  }

  return;
}
