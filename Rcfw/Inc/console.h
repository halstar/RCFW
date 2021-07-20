#ifndef __CONSOLE_H
#define __CONSOLE_H

#include "stm32f1xx_hal.h"

void CONSOLE_uartInit   (UART_HandleTypeDef *huart);
void CONSOLE_receiveData();

#endif /* __CONSOLE_H */
