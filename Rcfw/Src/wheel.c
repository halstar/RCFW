#include <math.h>

#include "wheel.h"

#include "stm32f1xx_hal.h"
#include "setup.h"
#include "chrono.h"
#include "log.h"

void WHL_init(T_WHL_Handle      *p_handle,
              char              *p_name,
              GPIO_TypeDef      *p_dirPin1Port,
              uint32_t           p_dirPin1,
              GPIO_TypeDef      *p_dirPin2Port,
              uint32_t           p_dirPin2,
              TIM_HandleTypeDef *p_pwmTimerHandle,
              uint32_t           p_pwmChannel,
              bool               p_invertOnUpdate,
              TIM_HandleTypeDef *p_encoderTimerHandle,
              bool               p_isMotorOn)
{
  LOG_info("Initializing wheel module for %s", p_name);

  p_handle->name = p_name;

  /* Setup motor (with a 0 speed & stopped direction, at this point) */
  MTR_init(&p_handle->motor,
            p_name,
            p_dirPin1Port,
            p_dirPin1,
            p_dirPin2Port,
            p_dirPin2,
            p_pwmTimerHandle,
            p_pwmChannel);

  /* Setup encoder */
  ENC_init(&p_handle->encoder,
            p_name,
            p_invertOnUpdate,
            p_encoderTimerHandle);

  /* Setup speed buffer */
  CBU_init(&p_handle->speedBuffer);

  /* Setup average speed */
  p_handle->averageSpeed = 0;

  /* Setup PID (with a target speed to 0) */
  PID_init(&p_handle->pid,
            p_name,
            STP_DRIVE_PID_P_FACTOR,
            STP_DRIVE_PID_I_FACTOR,
            STP_DRIVE_PID_D_FACTOR,
            0,
            STP_DRIVE_MIN_SPEED,
            STP_DRIVE_MAX_SPEED,
            STP_DRIVE_PID_ANTI_WIND_UP_FACTOR);

  p_handle->isMotorOn = p_isMotorOn;

  return;
}

void WHL_turnMotorOn(T_WHL_Handle *p_handle)
{
  p_handle->isMotorOn = true;

  return;
}

void WHL_turnMotorOff(T_WHL_Handle *p_handle)
{
  MTR_setSpeed(&p_handle->motor, 0);

  p_handle->isMotorOn = false;

  return;
}

void WHL_updateEncoder(T_WHL_Handle *p_handle,
                       uint32_t      p_count)
{
  ENC_update(&p_handle->encoder, p_count);

  return;
}

void WHL_setDirection(T_WHL_Handle *p_handle, T_MTR_DIRECTION p_direction)
{
  MTR_setDirection(&p_handle->motor, p_direction);

  return;
}

void WHL_setSpeed(T_WHL_Handle *p_handle, uint32_t p_speed)
{
  if (p_handle->isMotorOn == true)
  {
    MTR_setSpeed(&p_handle->motor, p_speed);
  }
  else
  {
    ; /* Nothing to do */
  }

  return;
}

void WHL_setPidKp(T_WHL_Handle *p_handle, float p_kp)
{
  PID_setKp(&p_handle->pid, p_kp);

  return;
}

void WHL_setPidKi(T_WHL_Handle *p_handle, float p_ki)
{
  PID_setKi(&p_handle->pid, p_ki);

  return;
}

void WHL_setPidKd(T_WHL_Handle *p_handle, float p_kd)
{
  PID_setKd(&p_handle->pid, p_kd);

  return;
}

void WHL_setPidTarget(T_WHL_Handle *p_handle, uint32_t p_speed)
{
  PID_setTargetValue(&p_handle->pid, p_speed);

  return;
}

void  WHL_setDirectTarget(T_WHL_Handle *p_handle, uint32_t p_speed)
{
  WHL_setSpeed(p_handle, p_speed);

  return;
}

void WHL_updateAverageSpeed(T_WHL_Handle *p_handle)
{
  uint32_t l_elapsedTimeInUs;
  float    l_measuredSpeed;

  l_elapsedTimeInUs = CHR_getTimeUs();

  l_measuredSpeed = (float)ENC_getCount(&p_handle->encoder) / (float)l_elapsedTimeInUs * (float)STP_DRIVE_PID_ENCODER_TO_SPEED_FACTOR;

  CBU_push(&p_handle->speedBuffer, l_measuredSpeed);

  p_handle->averageSpeed = CBU_getAverage(&p_handle->speedBuffer);

  return;
}

void WHL_updatePidSpeed(T_WHL_Handle *p_handle)
{
  uint32_t l_elapsedTimeInUs;
  float    l_pidSpeed;

  l_elapsedTimeInUs = CHR_getTimeUs();

  l_pidSpeed = PID_update(&p_handle->pid, fabs(p_handle->averageSpeed), l_elapsedTimeInUs);

  WHL_setSpeed(p_handle, l_pidSpeed);

  return;
}

float WHL_getAverageSpeed(T_WHL_Handle *p_handle)
{
  return p_handle->averageSpeed;
}

void WHL_logInfo(T_WHL_Handle *p_handle)
{
  T_MTR_DIRECTION l_direction;
  uint32_t        l_targetSpeed;
  uint32_t        l_actualSpeed;
  int32_t         l_count;

  l_direction   = MTR_getDirection  (&p_handle->motor  );
  l_targetSpeed = PID_getTargetValue(&p_handle->pid    );
  l_actualSpeed = MTR_getSpeed      (&p_handle->motor  );
  l_count       = ENC_getCount      (&p_handle->encoder);

  LOG_info("%s direction / target / speed / count / average: %2u / %2u / %2u / %2d / %2d",
           p_handle->name,
           l_direction,
           l_targetSpeed,
           l_actualSpeed,
           l_count,
      (int)p_handle->averageSpeed);

  return;
}
