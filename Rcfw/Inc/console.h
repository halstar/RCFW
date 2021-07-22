#ifndef __CONSOLE_H
#define __CONSOLE_H

#include "stm32f1xx_hal.h"

void CONSOLE_init       (UART_HandleTypeDef *p_uartHandle);
void CONSOLE_receiveData(void                            );

#endif /* __CONSOLE_H */
