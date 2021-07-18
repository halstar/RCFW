#include "console.h"

#include "stm32f1xx_hal.h"

uint8_t CONSOLE_rxBuffer[12] = {0};

void CONSOLE_receiveData(UART_HandleTypeDef *huart)
{
  HAL_UART_Receive_IT(huart, CONSOLE_rxBuffer, 1);

}
