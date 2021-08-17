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
  LOG_info("Initializing motor module for %s", p_name);

  p_handle->name           = p_name;
  p_handle->dirPin1Port    = p_dirPin1Port;
  p_handle->dirPin1        = p_dirPin1;
  p_handle->dirPin2Port    = p_dirPin2Port;
  p_handle->dirPin2        = p_dirPin2;
  p_handle->pwmTimerHandle = p_pwmTimerHandle;
  p_handle->pwmChannel     = p_pwmChannel;

  MTR_setDirection(p_handle, MTR_DIRECTION_STOP);
  MTR_setSpeed    (p_handle, 0                 );

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
    if (p_direction == MTR_DIRECTION_STOP)
    {
      LOG_debug("%s motor STOP", p_handle->name);

      HAL_GPIO_WritePin(p_handle->dirPin1Port, p_handle->dirPin1, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(p_handle->dirPin2Port, p_handle->dirPin2, GPIO_PIN_RESET);
    }
    else if (p_direction == MTR_DIRECTION_FORWARD)
    {
      LOG_debug("%s motor go FORWARD", p_handle->name);

      HAL_GPIO_WritePin(p_handle->dirPin1Port, p_handle->dirPin1, GPIO_PIN_SET  );
      HAL_GPIO_WritePin(p_handle->dirPin2Port, p_handle->dirPin2, GPIO_PIN_RESET);
    }
    else
    {
      LOG_debug("%s motor go BACKWARD", p_handle->name);

      HAL_GPIO_WritePin(p_handle->dirPin1Port, p_handle->dirPin1, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(p_handle->dirPin2Port, p_handle->dirPin2, GPIO_PIN_SET  );
    }

    p_handle->direction = p_direction;
  }

  return;
}

T_MTR_DIRECTION MTR_getDirection(T_MTR_Handle *p_handle)
{
  return p_handle->direction;
}

void MTR_setSpeed(T_MTR_Handle *p_handle, uint32_t p_speed)
{
  if (p_speed == p_handle->speed)
  {
    ; /* Nothing to do */
  }
  else
  {
    LOG_debug("Setting  %s motor speed to %u", p_handle->name, p_speed);

    __HAL_TIM_SET_COMPARE(p_handle->pwmTimerHandle, p_handle->pwmChannel, p_speed);

    p_handle->speed = p_speed;
  }

  return;
}

uint32_t MTR_getSpeed(T_MTR_Handle *p_handle)
{
  return p_handle->speed;
}

void MTR_logInfo(T_MTR_Handle *p_handle)
{
  if (p_handle->direction == MTR_DIRECTION_STOP)
  {
    LOG_debug("%s motor direction: STOP", p_handle->name);
  }
  else if (p_handle->direction == MTR_DIRECTION_FORWARD)
  {
    LOG_debug("%s motor direction: FORWARD", p_handle->name);
  }
  else if (p_handle->direction == MTR_DIRECTION_BACKWARD)
  {
    LOG_debug("%s motor direction: BACKWARD", p_handle->name);
  }
  else
  {
    LOG_error("%s motor unsupported direction: %u", p_handle->name, p_handle->direction);
  }

  LOG_debug("%s motor speed    : %u", p_handle->name, p_handle->speed);

  return;
}
