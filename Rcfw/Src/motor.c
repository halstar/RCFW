#include "motor.h"

#include "stm32f1xx_hal.h"
#include "log.h"

void MTR_init(T_MTR_Handle      *p_handle,
              char              *p_name,
              GPIO_TypeDef      *p_dirPin1Port,
              uint32_t           p_dirPin1,
              GPIO_TypeDef      *p_dirPin2Port,
              uint32_t           p_dirPin2,
              TIM_HandleTypeDef *p_pwmTimerHandle,
              uint32_t           p_pwmChannel)
{
  LOG_info("Initializing Motor module for %s", p_name);

  p_handle->name           = p_name;
  p_handle->dirPin1Port    = p_dirPin1Port;
  p_handle->dirPin1        = p_dirPin1;
  p_handle->dirPin2Port    = p_dirPin2Port;
  p_handle->dirPin2        = p_dirPin2;
  p_handle->pwmTimerHandle = p_pwmTimerHandle;
  p_handle->pwmChannel     = p_pwmChannel;

  MTR_setDirection(p_handle, MTR_DIRECTION_FORWARD);
  MTR_setSpeed    (p_handle, 0                    );
  MTR_stop        (p_handle                       );

  return;
}

void MTR_setDirection(T_MTR_Handle *p_handle, T_MTR_DIRECTION p_direction)
{
  if (p_direction == p_handle->direction)
  {
    ; /* Nothing to do */
  }
  else
  {
    if (p_direction == MTR_DIRECTION_FORWARD)
    {
      LOG_debug("%s motor go FORWARD", p_handle->name);

      HAL_GPIO_WritePin(p_handle->dirPin1Port, p_handle->dirPin1, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(p_handle->dirPin2Port, p_handle->dirPin2, GPIO_PIN_SET  );
    }
    else
    {
      LOG_debug("%s motor go BACKWARD", p_handle->name);

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
