#include "console.h"

#include <stdio.h>

#include "stm32f1xx_hal.h"

UART_HandleTypeDef *CONSOLE_uartHandle;
uint8_t CONSOLE_rxBuffer[12] = {0};

void CONSOLE_uartInit(UART_HandleTypeDef *huart)
{
  CONSOLE_uartHandle = huart;

  return;
}

void CONSOLE_receiveData()
{
  HAL_UART_Receive_IT(CONSOLE_uartHandle, CONSOLE_rxBuffer, 12);

  /* Resend received data (echo) */
  HAL_UART_Transmit(CONSOLE_uartHandle, CONSOLE_rxBuffer, 12, 100);

  return;
}

int fputc(int character, FILE *fileDescriptor)
{
  HAL_UART_Transmit(CONSOLE_uartHandle, (uint8_t *)&character, 1, 100);

  return character;
}
