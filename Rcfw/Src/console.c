#include <stdio.h>

#include "console.h"

#include "stm32f1xx_hal.h"
#include "log.h"

static UART_HandleTypeDef *CONSOLE_uartHandle;
static uint8_t             CONSOLE_rxBuffer[12] = {0};

void CONSOLE_init(UART_HandleTypeDef *p_uartHandle)
{
  LOG_info("Initializing console");

  CONSOLE_uartHandle = p_uartHandle;

  return;
}

void CONSOLE_receiveData(void)
{
  HAL_UART_Receive_IT(CONSOLE_uartHandle, CONSOLE_rxBuffer, 12);

  /* Resend received data (echo) */
//  HAL_UART_Transmit(CONSOLE_uartHandle, CONSOLE_rxBuffer, 12, 100);

  return;
}

int fputc(int p_character, FILE *p_fileDescriptor)
{
  HAL_UART_Transmit(CONSOLE_uartHandle, (uint8_t *)&p_character, 1, 1000);

  return p_character;
}
