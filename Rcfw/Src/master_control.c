#include "master_control.h"

#include "stm32f1xx_hal.h"

static uint8_t MASTER_CONTROL_rxBuffer[12] = {0};

void MASTER_CONTROL_receiveData(UART_HandleTypeDef *p_huart)
{
  HAL_UART_Receive_IT(p_huart, MASTER_CONTROL_rxBuffer, 1);

  return;
}

