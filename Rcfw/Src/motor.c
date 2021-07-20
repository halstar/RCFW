#include "motor.h"

#include <string.h>

#include "stm32f1xx_hal.h"

#include "log.h"

void MOTOR_init(MOTOR_Handle *handle, char *name)
{
  LOG_info("Initializing Motor module for %s", name);

  (void)strncpy((char *)handle->name, name, MOTOR_NAME_MAX_LENGTH);

  MOTOR_setSpeed(handle, 0);
  MOTOR_stop    (handle   );

  return;
}

void MOTOR_setDirection(MOTOR_Handle *handle, uint32_t direction)
{
  LOG_debug("Setting %s motor direction to %u", handle->name, direction);

  if (direction == MOTOR_DIRECTION_FORWARD)
  {
    HAL_GPIO_WritePin(handle->dirPin1Port, handle->dirPin1, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(handle->dirPin2Port, handle->dirPin2, GPIO_PIN_SET  );
  }
  else
  {
    HAL_GPIO_WritePin(handle->dirPin1Port, handle->dirPin1, GPIO_PIN_SET  );
    HAL_GPIO_WritePin(handle->dirPin2Port, handle->dirPin2, GPIO_PIN_RESET);
  }

  handle->direction = direction;

  return;
}

void MOTOR_setSpeed(MOTOR_Handle *handle, uint32_t speed)
{
  LOG_debug("Setting %s motor speed to %u", handle->name, speed);

  __HAL_TIM_SET_COMPARE(handle->pwmTimerHandle, handle->pwmChannel, speed);

  handle->speed = speed;

  return;
}

void MOTOR_start(MOTOR_Handle *handle)
{
  LOG_info("Starting %s motor", handle->name);

  MOTOR_setDirection(handle, handle->direction);

  return;
}

void MOTOR_stop(MOTOR_Handle *handle)
{
  LOG_info("Stopping %s", handle->name);

  HAL_GPIO_WritePin(handle->dirPin1Port, handle->dirPin1, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(handle->dirPin2Port, handle->dirPin2, GPIO_PIN_RESET);

  return;
}
