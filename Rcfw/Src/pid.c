#include <stdlib.h>

#include "pid.h"

#include "utils.h"
#include "log.h"

void PID_init(T_PID_Handle *p_handle, int32_t p_kp, int32_t p_ki, int32_t p_kd, int32_t p_targetValue, int32_t p_minValue, int32_t p_maxValue, int32_t p_antiWindUpFactor)
{
  LOG_info("Initializing PID");

  p_handle->kp              = p_kp;
  p_handle->ki              = p_ki;
  p_handle->kd              = p_kd;
  p_handle->targetValue     = p_targetValue;
  p_handle->minValue        = p_minValue;
  p_handle->maxValue        = p_maxValue;
  p_handle->antiWindUpValue = p_antiWindUpFactor * UTI_MAX(abs(p_handle->minValue), abs(p_handle->maxValue));

  PID_reset  (p_handle);
  PID_logInfo(p_handle);

  return;
}

void PID_reset(T_PID_Handle *p_handle)
{
  p_handle->pValue        = 0;
  p_handle->iValue        = 0;
  p_handle->dValue        = 0;
  p_handle->computedValue = 0;
  p_handle->lastError     = 0;

  return;
}

void PID_setKp(T_PID_Handle *p_handle, int32_t p_kp)
{
  p_handle->kp = p_kp;

  return;
}

void PID_setKi(T_PID_Handle *p_handle, int32_t p_ki)
{
  p_handle->ki = p_ki;

  return;
}

void PID_setKd(T_PID_Handle *p_handle, int32_t p_kd)
{
  p_handle->kd = p_kd;

  return;
}

void PID_setTargetValue(T_PID_Handle *p_handle, int32_t p_targetValue)
{
  p_handle->targetValue = p_targetValue;

  return;
}

void PID_setMinValue(T_PID_Handle *p_handle, int32_t p_minValue)
{
  p_handle->minValue = p_minValue;

  return;
}

void PID_setMaxValue(T_PID_Handle *p_handle, int32_t p_maxValue)
{
  p_handle->maxValue = p_maxValue;

  return;
}

void PID_setAntiWindUp(T_PID_Handle *p_handle, int32_t p_antiWindUpFactor)
{
  p_handle->antiWindUpValue = p_antiWindUpFactor * UTI_MAX(abs(p_handle->minValue), abs(p_handle->maxValue));

  return;
}

int32_t PID_getKp(T_PID_Handle *p_handle)
{
  return p_handle->kp;
}

int32_t PID_getKi (T_PID_Handle *p_handle)
{
  return p_handle->ki;
}

int32_t PID_getKd (T_PID_Handle *p_handle)
{
  return p_handle->kd;
}

int32_t PID_getTargetValue(T_PID_Handle *p_handle)
{
  return p_handle->targetValue;
}

int32_t PID_getMinValue(T_PID_Handle *p_handle)
{
  return p_handle->minValue;
}

int32_t PID_getMaxValue(T_PID_Handle *p_handle)
{
  return p_handle->maxValue;
}

int32_t PID_getAntiWindUp(T_PID_Handle *p_handle)
{
  return p_handle->antiWindUpValue;
}

int32_t PID_update(T_PID_Handle *p_handle, int32_t p_currentValue, uint32_t p_timeDelta)
{
  int32_t l_currentError;

  l_currentError = p_handle->targetValue - p_currentValue;

  p_handle->pValue    =  l_currentError;
  p_handle->iValue   +=  l_currentError * p_timeDelta;
  p_handle->iValue    =  UTI_clampIntValue(p_handle->iValue, -p_handle->antiWindUpValue, p_handle->antiWindUpValue, false, 0);
  p_handle->dValue    = (l_currentError - p_handle->lastError) / p_timeDelta;
  p_handle->lastError =  l_currentError;

  p_handle->computedValue = p_handle->kp * p_handle->pValue + p_handle->ki * p_handle->iValue + p_handle->kd * p_handle->dValue;
  p_handle->computedValue = UTI_clampIntValue(p_handle->computedValue, p_handle->minValue, p_handle->maxValue, false, 0);

  return p_currentValue;
}

void PID_logInfo(T_PID_Handle *p_handle)
{
  LOG_info(" kp = %4d -  ki = %4d -     kd = %4d", p_handle->kp         , p_handle->ki      , p_handle->kd             );
  LOG_info("  p = %4d -   i = %4d -      d = %4d", p_handle->pValue     , p_handle->iValue  , p_handle->dValue         );
  LOG_info("min = %4d - max = %4d - a.w.up = %4d", p_handle->minValue   , p_handle->maxValue, p_handle->antiWindUpValue);
  LOG_info("  t = %4d - val = %4d               ", p_handle->targetValue, p_handle->computedValue                      );

  return;
}

