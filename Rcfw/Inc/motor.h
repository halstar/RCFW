#ifndef __MOTOR_H
#define __MOTOR_H

#include "stm32f1xx_hal.h"

#define MTR_NAME_MAX_LENGTH 12

#define MTR_DIRECTION_FORWARD  0
#define MTR_DIRECTION_BACKWARD 1

typedef struct
{
  uint8_t            name[MTR_NAME_MAX_LENGTH];
  GPIO_TypeDef      *dirPin1Port;
  GPIO_TypeDef      *dirPin2Port;
  uint32_t           dirPin1;
  uint32_t           dirPin2;
  uint32_t           direction;
  uint32_t           speed;
  TIM_HandleTypeDef *pwmTimerHandle;
  uint32_t           pwmChannel;
} T_MTR_Handle;

void MTR_init        (T_MTR_Handle *p_handle, char    *p_name     );
void MTR_setDirection(T_MTR_Handle *p_handle, uint32_t p_direction);
void MTR_setSpeed    (T_MTR_Handle *p_handle, uint32_t p_speed    );
void MTR_start       (T_MTR_Handle *p_handle                      );
void MTR_stop        (T_MTR_Handle *p_handle                      );

#endif /* __MOTOR_H */
