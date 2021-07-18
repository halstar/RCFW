#include "master_control.h"

#include "stm32f1xx_hal.h"

uint8_t MASTER_CONTROL_rxBuffer[12] = {0};

void MASTER_CONTROL_receiveData(UART_HandleTypeDef *huart)
{
  HAL_UART_Receive_IT(huart, MASTER_CONTROL_rxBuffer, 1);

}

