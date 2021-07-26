#include <string.h>

#include "motor.h"

#include "stm32f1xx_hal.h"
#include "log.h"

void MTR_init(T_MTR_Handle *p_handle, char *p_name)
{
  LOG_info("Initializing Motor module for %s", p_name);

  (void)strncpy((char *)p_handle->name, p_name, MTR_NAME_MAX_LENGTH);

  MTR_setSpeed(p_handle, 0);
  MTR_stop    (p_handle   );

  return;
}

void MTR_setDirection(T_MTR_Handle *p_handle, uint32_t p_direction)
{
  if (p_direction == p_handle->direction)
  {
    ; /* Nothing to do */
  }
  else
  {
    LOG_debug("Setting  %s motor direction to %u", p_handle->name, p_direction);

    if (p_direction == MTR_DIRECTION_FORWARD)
    {
      HAL_GPIO_WritePin(p_handle->dirPin1Port, p_handle->dirPin1, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(p_handle->dirPin2Port, p_handle->dirPin2, GPIO_PIN_SET  );
    }
    else
    {
      HAL_GPIO_WritePin(p_handle->dirPin1Port, p_handle->dirPin1, GPIO_PIN_SET  );
      HAL_GPIO_WritePin(p_handle->dirPin2Port, p_handle->dirPin2, GPIO_PIN_RESET);
    }

    p_handle->direction = p_direction;
  }

  return;
}

void MTR_setSpeed(T_MTR_Handle *p_handle, uint32_t p_speed)
{
  if (p_speed == p_handle->speed)
  {
    ; /* Nothing to do */
  }
  else
  {
    LOG_debug("Setting  %s motor p_speed to %u", p_handle->name, p_speed);

    __HAL_TIM_SET_COMPARE(p_handle->pwmTimerHandle, p_handle->pwmChannel, p_speed);

    p_handle->speed = p_speed;
  }

  return;
}

void MTR_start(T_MTR_Handle *p_handle)
{
  LOG_info("Starting %s motor", p_handle->name);

  MTR_setDirection(p_handle, p_handle->direction);

  return;
}

void MTR_stop(T_MTR_Handle *p_handle)
{
  LOG_info("Stopping %s", p_handle->name);

  HAL_GPIO_WritePin(p_handle->dirPin1Port, p_handle->dirPin1, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(p_handle->dirPin2Port, p_handle->dirPin2, GPIO_PIN_RESET);

  return;
}
