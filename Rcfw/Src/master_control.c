#include "master_control.h"

#include "stm32f1xx_hal.h"

static uint8_t MAS_rxBuffer[12] = {0};

void MAS_receiveData(UART_HandleTypeDef *p_huart)
{
  HAL_UART_Receive_IT(p_huart, MAS_rxBuffer, 1);

  return;
}

