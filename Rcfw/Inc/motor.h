#ifndef __MOTOR_H
#define __MOTOR_H

#include "stm32f1xx_hal.h"

typedef enum
{
  MTR_DIRECTION_FORWARD = 0,
  MTR_DIRECTION_BACKWARD
} T_MTR_DIRECTION;

typedef struct
{
  char              *name;
  GPIO_TypeDef      *dirPin1Port;
  uint32_t           dirPin1;
  GPIO_TypeDef      *dirPin2Port;
  uint32_t           dirPin2;
  T_MTR_DIRECTION    direction;
  uint32_t           speed;
  TIM_HandleTypeDef *pwmTimerHandle;
  uint32_t           pwmChannel;
} T_MTR_Handle;

void MTR_init(T_MTR_Handle      *p_handle,
              char              *p_name,
              GPIO_TypeDef      *p_dirPin1Port,
              uint32_t           p_dirPin1,
              GPIO_TypeDef      *p_dirPin2Port,
              uint32_t           p_dirPin2,
              TIM_HandleTypeDef *p_pwmTimerHandle,
              uint32_t           p_pwmChannel);

void MTR_setDirection(T_MTR_Handle *p_handle, T_MTR_DIRECTION p_direction);
void MTR_setSpeed    (T_MTR_Handle *p_handle, uint32_t        p_speed    );
void MTR_start       (T_MTR_Handle *p_handle                             );
void MTR_stop        (T_MTR_Handle *p_handle                             );

#endif /* __MOTOR_H */
