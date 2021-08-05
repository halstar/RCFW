#ifndef __MASTER_CONTROL_H
#define __MASTER_CONTROL_H

#include "stm32f1xx_hal.h"
#include "string_fifo.h"

#define MAS_MAX_STRING_LENGTH 32

void MAS_init       (UART_HandleTypeDef *p_uartHandle);
void MAS_receiveData(void                            );
void MAS_updateFifo (T_SFO_Context      *p_fifo      );
void MAS_sendString (char               *p_string    );

#endif /* __MASTER_CONTROL_H */
