#ifndef __BLUETOOTH_CONTROL_H
#define __BLUETOOTH_CONTROL_H

#include "stm32f1xx_hal.h"

#define KEY_NONE            0
#define KEY_SELECT          1
#define KEY_L3              2
#define KEY_R3              3
#define KEY_START           4
#define KEY_PAD_UP          5
#define KEY_PAD_RIGHT       6
#define KEY_PAD_DOWN        7
#define KEY_PAD_LEFT        8
#define KEY_L2              9
#define KEY_R2             10
#define KEY_L1             11
#define KEY_R1             12
#define KEY_GREEN_TRIANGLE 13
#define KEY_RED_CIRCLE     14
#define KEY_BLUE_CROSS     15
#define KEY_PINK_SQUARE    16

typedef struct {
  uint32_t leftX;
  uint32_t leftY;
  uint32_t rightX;
  uint32_t rightY;
  uint32_t key;
} BLUETOOTH_CONTROL_DATA;

void BLUETOOTH_CONTROL_init       (void);
void BLUETOOTH_CONTROL_receiveData(BLUETOOTH_CONTROL_DATA *data);

#endif /* __BLUETOOTH_CONTROL_H */
