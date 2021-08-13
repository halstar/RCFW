#include <stdlib.h>

#include "pid.h"

#include "utils.h"
#include "log.h"

void  PID_init(T_PID_Handle *p_handle,
               char         *p_name,
               float         p_kp,
               float         p_ki,
               float         p_kd,
               float         p_targetValue,
               float         p_minValue,
               float         p_maxValue,
               float         p_antiWindUpFactor)
{
  LOG_info("Initializing PID module for %s", p_name);

  p_handle->name            = p_name;
  p_handle->kp              = p_kp;
  p_handle->ki              = p_ki;
  p_handle->kd              = p_kd;
  p_handle->targetValue     = p_targetValue;
  p_handle->minValue        = p_minValue;
  p_handle->maxValue        = p_maxValue;
  p_handle->antiWindUpValue = p_antiWindUpFactor * UTI_MAX(abs(p_handle->minValue), abs(p_handle->maxValue));

  PID_reset  (p_handle       );
  PID_logInfo(p_handle, false);

  return;
}

void PID_reset(T_PID_Handle *p_handle)
{
  p_handle->pValue        = 0.0f;
  p_handle->iValue        = 0.0f;
  p_handle->dValue        = 0.0f;
  p_handle->computedValue = 0.0f;
  p_handle->lastError     = 0.0f;

  return;
}

void PID_setKp(T_PID_Handle *p_handle, float p_kp)
{
  p_handle->kp = p_kp;

  return;
}

void PID_setKi(T_PID_Handle *p_handle, float p_ki)
{
  p_handle->ki = p_ki;

  return;
}

void PID_setKd(T_PID_Handle *p_handle, float p_kd)
{
  p_handle->kd = p_kd;

  return;
}

void PID_setTargetValue(T_PID_Handle *p_handle, float p_targetValue)
{
  p_handle->targetValue = p_targetValue;

  return;
}

void PID_setMinValue(T_PID_Handle *p_handle, float p_minValue)
{
  p_handle->minValue = p_minValue;

  return;
}

void PID_setMaxValue(T_PID_Handle *p_handle, float p_maxValue)
{
  p_handle->maxValue = p_maxValue;

  return;
}

void PID_setAntiWindUpFactor(T_PID_Handle *p_handle, float p_antiWindUpFactor)
{
  p_handle->antiWindUpValue = p_antiWindUpFactor * UTI_MAX(abs(p_handle->minValue), abs(p_handle->maxValue));

  return;
}

float PID_getKp(T_PID_Handle *p_handle)
{
  return p_handle->kp;
}

float PID_getKi (T_PID_Handle *p_handle)
{
  return p_handle->ki;
}

float PID_getKd (T_PID_Handle *p_handle)
{
  return p_handle->kd;
}

float PID_getTargetValue(T_PID_Handle *p_handle)
{
  return p_handle->targetValue;
}

float PID_getMinValue(T_PID_Handle *p_handle)
{
  return p_handle->minValue;
}

float PID_getMaxValue(T_PID_Handle *p_handle)
{
  return p_handle->maxValue;
}

float PID_getAntiWindUpValue(T_PID_Handle *p_handle)
{
  return p_handle->antiWindUpValue;
}

float PID_update(T_PID_Handle *p_handle, float p_currentValue, uint32_t p_timeDelta)
{
  int32_t l_currentError;

  l_currentError = p_handle->targetValue - p_currentValue;

  p_handle->pValue    =  l_currentError;
  p_handle->iValue   +=  l_currentError * p_timeDelta;
  p_handle->iValue    =  UTI_clampIntValue(p_handle->iValue, -p_handle->antiWindUpValue, p_handle->antiWindUpValue, true, 0);
  p_handle->dValue    = (l_currentError - p_handle->lastError) / p_timeDelta;
  p_handle->lastError =  l_currentError;

  p_handle->computedValue = p_handle->kp * p_handle->pValue + p_handle->ki * p_handle->iValue + p_handle->kd * p_handle->dValue;
  p_handle->computedValue = UTI_clampIntValue(p_handle->computedValue, p_handle->minValue, p_handle->maxValue, true, 0);

  return p_handle->computedValue;
}

void PID_logInfo(T_PID_Handle *p_handle, bool p_compactLog)
{
  if (p_compactLog == false)
  {
    LOG_info("% PID detailed data:", p_handle->name);
    LOG_info(" kp = %6.2f -  ki = %6.2f -     kd = %6.2f", p_handle->kp         , p_handle->ki      , p_handle->kd             );
    LOG_info("  p = %6.2f -   i = %6.2f -      d = %6.2f", p_handle->pValue     , p_handle->iValue  , p_handle->dValue         );
    LOG_info("min = %6.2f - max = %6.2f - a.w.up = %6.2f", p_handle->minValue   , p_handle->maxValue, p_handle->antiWindUpValue);
    LOG_info("  t = %6.2f - val = %6.2f               "  , p_handle->targetValue, p_handle->computedValue                      );
  }
  else
  {
    LOG_info("%s PID: p = %6.2f - i = %6.2f - d = %6.2f - t = %6.2f - val = %6.2f",
             p_handle->name,
             p_handle->pValue,
             p_handle->iValue,
             p_handle->dValue,
             p_handle->targetValue,
             p_handle->computedValue);
  }

  return;
}

