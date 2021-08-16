#ifndef __CONSOLE_H
#define __CONSOLE_H

#include "stm32f1xx_hal.h"
#include "string_fifo.h"

void CON_init       (UART_HandleTypeDef *p_uartHandle               );
void CON_receiveData(void                                           );
void CON_updateFifo (T_SFO_Handle       *p_fifo                     );
void CON_sendString (char               *p_string, uint32_t p_length);

#endif /* __CONSOLE_H */
