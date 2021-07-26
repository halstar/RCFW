#include <stdio.h>

#include "console.h"

#include "stm32f1xx_hal.h"
#include "log.h"

static UART_HandleTypeDef *CON_uartHandle;
static uint8_t             CON_rxBuffer[12] = {0};

void CON_init(UART_HandleTypeDef *p_uartHandle)
{
  LOG_info("Initializing console");

  CON_uartHandle = p_uartHandle;

  return;
}

void CON_receiveData(void)
{
  HAL_UART_Receive_IT(CON_uartHandle, CON_rxBuffer, 12);

  /* Resend received data (echo) */
//  HAL_UART_Transmit(CON_uartHandle, CON_rxBuffer, 12, 100);

  return;
}

int fputc(int p_character, FILE *p_fileDescriptor)
{
  HAL_UART_Transmit(CON_uartHandle, (uint8_t *)&p_character, 1, 1000);

  return p_character;
}
