#ifndef __MOTOR_H
#define __MOTOR_H

#include "stm32f1xx_hal.h"

#define MOTOR_NAME_MAX_LENGTH 12

#define MOTOR_DIRECTION_FORWARD  0
#define MOTOR_DIRECTION_BACKWARD 1

typedef struct
{
  uint8_t            name[MOTOR_NAME_MAX_LENGTH];
  GPIO_TypeDef      *dirPin1Port;
  GPIO_TypeDef      *dirPin2Port;
  uint32_t           dirPin1;
  uint32_t           dirPin2;
  uint32_t           direction;
  uint32_t           speed;
  TIM_HandleTypeDef *pwmTimerHandle;
  uint32_t           pwmChannel;
} MOTOR_Handle;

void MOTOR_init        (MOTOR_Handle *handle, char *name);
void MOTOR_setDirection(MOTOR_Handle *handle, uint32_t direction);
void MOTOR_setSpeed    (MOTOR_Handle *handle, uint32_t speed);
void MOTOR_start       (MOTOR_Handle *handle);
void MOTOR_stop        (MOTOR_Handle *handle);

#endif /* __MOTOR_H */
