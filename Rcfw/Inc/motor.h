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
} T_MOTOR_Handle;

void MOTOR_init        (T_MOTOR_Handle *p_handle, char    *p_name     );
void MOTOR_setDirection(T_MOTOR_Handle *p_handle, uint32_t p_direction);
void MOTOR_setSpeed    (T_MOTOR_Handle *p_handle, uint32_t p_speed    );
void MOTOR_start       (T_MOTOR_Handle *p_handle                      );
void MOTOR_stop        (T_MOTOR_Handle *p_handle                      );

#endif /* __MOTOR_H */
