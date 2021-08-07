#ifndef __BLUTOOTH_CONTROL_H
#define __BLUTOOTH_CONTROL_H

#include "stm32f1xx_hal.h"

typedef enum
{
  BLU_BUTTON_NONE       = 0,
  BLU_BUTTON_SELECT        ,
  BLU_BUTTON_L3            ,
  BLU_BUTTON_R3            ,
  BLU_BUTTON_START         ,
  BLU_BUTTON_PAD_UP        ,
  BLU_BUTTON_PAD_RIGHT     ,
  BLU_BUTTON_PAD_DOWN      ,
  BLU_BUTTON_PAD_LEFT      ,
  BLU_BUTTON_L2            ,
  BLU_BUTTON_R2            ,
  BLU_BUTTON_L1            ,
  BLU_BUTTON_R1            ,
  BLU_BUTTON_GREEN_TRIANGLE,
  BLU_BUTTON_RED_CIRCLE    ,
  BLU_BUTTON_BLUE_CROSS    ,
  BLU_BUTTON_PINK_SQUARE   ,
  BLU_BUTTON_COUNT_VALUE   ,
} T_BLU_BUTTON;

typedef struct T_BLU_Data
{
  int32_t      leftX;
  int32_t      leftY;
  int32_t      rightX;
  int32_t      rightY;
  T_BLU_BUTTON button;
} T_BLU_Data;

void BLU_init       (void              );
void BLU_initData   (T_BLU_Data *p_data);
void BLU_receiveData(T_BLU_Data *p_data);

#endif /* __BLUTOOTH_CONTROL_H */
