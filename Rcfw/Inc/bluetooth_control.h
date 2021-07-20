#ifndef __BLUETOOTH_CONTROL_H
#define __BLUETOOTH_CONTROL_H

#include "stm32f1xx_hal.h"

#define PSB_SELECT          1
#define PSB_L3              2
#define PSB_R3              3
#define PSB_START           4
#define PSB_PAD_UP          5
#define PSB_PAD_RIGHT       6
#define PSB_PAD_DOWN        7
#define PSB_PAD_LEFT        8
#define PSB_L2              9
#define PSB_R2             10
#define PSB_L1             11
#define PSB_R1             12
#define PSB_GREEN_TRIANGLE 13
#define PSB_RED_CIRCLE     14
#define PSB_BLUE_CROSS     15
#define PSB_PINK_SQUARE    16

extern uint8_t PS2_LX, PS2_LY, PS2_RX, PS2_RY, PS2_KEY;

void BLUETOOTH_CONTROL_init(void);
void BLUETOOTH_CONTROL_receiveData(void);

#endif /* __BLUETOOTH_CONTROL_H */
