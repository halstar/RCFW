#ifndef __MOTOR_H
#define __MOTOR_H

#include "stm32f1xx_hal.h"

typedef enum
{
  MTR_DIRECTION_STOP = 0,
  MTR_DIRECTION_FORWARD,
  MTR_DIRECTION_BACKWARD
} T_MTR_DIRECTION;

typedef struct T_MTR_Handle
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

void            MTR_setDirection(T_MTR_Handle *p_handle, T_MTR_DIRECTION p_direction);
T_MTR_DIRECTION MTR_getDirection(T_MTR_Handle *p_handle                             );
void            MTR_setSpeed    (T_MTR_Handle *p_handle, uint32_t        p_speed    );
uint32_t        MTR_getSpeed    (T_MTR_Handle *p_handle                             );
void            MTR_logInfo     (T_MTR_Handle *p_handle                             );

#endif /* __MOTOR_H */
