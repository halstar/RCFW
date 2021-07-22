#include <stdio.h>

#include "console.h"

#include "stm32f1xx_hal.h"

static UART_HandleTypeDef *CONSOLE_uartHandle;
static uint8_t CONSOLE_rxBuffer[12] = {0};

void CONSOLE_uartInit(UART_HandleTypeDef *huart)
{
  CONSOLE_uartHandle = huart;

  return;
}

void CONSOLE_receiveData()
{
  HAL_UART_Receive_IT(CONSOLE_uartHandle, CONSOLE_rxBuffer, 12);

  /* Resend received data (echo) */
//  HAL_UART_Transmit(CONSOLE_uartHandle, CONSOLE_rxBuffer, 12, 100);

  return;
}

int fputc(int character, FILE *fileDescriptor)
{
  HAL_UART_Transmit(CONSOLE_uartHandle, (uint8_t *)&character, 1, 1000);

  return character;
}
