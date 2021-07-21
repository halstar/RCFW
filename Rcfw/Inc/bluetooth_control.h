#ifndef __BLUETOOTH_CONTROL_H
#define __BLUETOOTH_CONTROL_H

#include "stm32f1xx_hal.h"

#define BUTTON_NONE            0
#define BUTTON_SELECT          1
#define BUTTON_L3              2
#define BUTTON_R3              3
#define BUTTON_START           4
#define BUTTON_PAD_UP          5
#define BUTTON_PAD_RIGHT       6
#define BUTTON_PAD_DOWN        7
#define BUTTON_PAD_LEFT        8
#define BUTTON_L2              9
#define BUTTON_R2             10
#define BUTTON_L1             11
#define BUTTON_R1             12
#define BUTTON_GREEN_TRIANGLE 13
#define BUTTON_RED_CIRCLE     14
#define BUTTON_BLUE_CROSS     15
#define BUTTON_PINK_SQUARE    16

typedef struct {
  int32_t  leftX;
  int32_t  leftY;
  int32_t  rightX;
  int32_t  rightY;
  uint32_t button;
} BLUETOOTH_CONTROL_DATA;

void BLUETOOTH_CONTROL_init       (void);
void BLUETOOTH_CONTROL_receiveData(BLUETOOTH_CONTROL_DATA *data);

#endif /* __BLUETOOTH_CONTROL_H */
