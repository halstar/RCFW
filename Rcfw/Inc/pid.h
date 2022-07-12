#ifndef __PID_H
#define __PID_H

#include <stdbool.h>

#include "stm32f1xx_hal.h"

typedef struct T_PID_Handle
{
  char *name;
  /* Configuration values */
  float kp;
  float ki;
  float kd;
  float targetValue;
  float minValue;
  float maxValue;
  float antiWindUpValue;
  /* Computation values */
  float pValue;
  float iValue;
  float dValue;
  float computedValue;
  float lastError;
} T_PID_Handle;

void PID_init(T_PID_Handle *p_handle,
               char         *p_name,
               float         p_kp,
               float         p_ki,
               float         p_kd,
               float         p_targetValue,
               float         p_minValue,
               float         p_maxValue,
               float         p_antiWindUpFactor);

void  PID_reset              (T_PID_Handle *p_handle);
void  PID_setKp              (T_PID_Handle *p_handle, float p_kp);
void  PID_setKi              (T_PID_Handle *p_handle, float p_ki);
void  PID_setKd              (T_PID_Handle *p_handle, float p_kd);
void  PID_setTargetValue     (T_PID_Handle *p_handle, float p_targetValue);
void  PID_setMinValue        (T_PID_Handle *p_handle, float p_minValue);
void  PID_setMaxValue        (T_PID_Handle *p_handle, float p_maxValue);
void  PID_setAntiWindUpFactor(T_PID_Handle *p_handle, float p_antiWindUpFactor);
float PID_getKp              (T_PID_Handle *p_handle);
float PID_getKi              (T_PID_Handle *p_handle);
float PID_getKd              (T_PID_Handle *p_handle);
float PID_getTargetValue     (T_PID_Handle *p_handle);
float PID_getMinValue        (T_PID_Handle *p_handle);
float PID_getMaxValue        (T_PID_Handle *p_handle);
float PID_getAntiWindUpValue (T_PID_Handle *p_handle);
float PID_update             (T_PID_Handle *p_handle, float p_currentValue, uint32_t p_timeDelta);
void  PID_logInfo            (T_PID_Handle *p_handle, bool p_compactLog);

#endif /* __PID_H */
