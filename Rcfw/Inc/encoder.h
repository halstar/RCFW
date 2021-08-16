#ifndef __ENCODER_H
#define __ENCODER_H

#include <stdbool.h>

#include "stm32f1xx_hal.h"

typedef struct T_ENC_Handle
{
  char              *name;
  int32_t            value;
  bool               invertOnUpdate;
  TIM_HandleTypeDef *timerHandle;
} T_ENC_Handle;

void ENC_init(T_ENC_Handle      *p_handle,
              char              *p_name,
              bool               p_invertOnUpdate,
              TIM_HandleTypeDef *p_encoderTimerHandle);

void    ENC_reset   (T_ENC_Handle *p_handle                 );
void    ENC_update  (T_ENC_Handle *p_handle, int32_t p_value);
int32_t ENC_getCount(T_ENC_Handle *p_handle                 );
void    ENC_logInfo (T_ENC_Handle *p_handle                 );

#endif /* __ENCODER_H */
