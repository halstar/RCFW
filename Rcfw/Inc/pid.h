#ifndef __PID_H
#define __PID_H

#include "stm32f1xx_hal.h"

typedef struct
{
  /* Configuration values */
  int32_t kp;
  int32_t ki;
  int32_t kd;
  int32_t targetValue;
  int32_t minValue;
  int32_t maxValue;
  int32_t antiWindUpValue;
  /* Computation values */
  int32_t pValue;
  int32_t iValue;
  int32_t dValue;
  int32_t computedValue;
  int32_t lastError;
} T_PID_Handle;

void    PID_init          (T_PID_Handle *p_handle, int32_t p_kp, int32_t p_ki, int32_t p_kd, int32_t p_targetValue, int32_t p_minValue, int32_t p_maxValue, int32_t p_antiWindUpFactor);
void    PID_reset         (T_PID_Handle *p_handle);
void    PID_setKp         (T_PID_Handle *p_handle, int32_t p_kp);
void    PID_setKi         (T_PID_Handle *p_handle, int32_t p_ki);
void    PID_setKd         (T_PID_Handle *p_handle, int32_t p_kd);
void    PID_setTargetValue(T_PID_Handle *p_handle, int32_t p_targetValue);
void    PID_setMinValue   (T_PID_Handle *p_handle, int32_t p_minValue);
void    PID_setMaxValue   (T_PID_Handle *p_handle, int32_t p_maxValue);
void    PID_setAntiWindUp (T_PID_Handle *p_handle, int32_t p_antiWindUpFactor);
int32_t PID_getKp         (T_PID_Handle *p_handle);
int32_t PID_getKi         (T_PID_Handle *p_handle);
int32_t PID_getKd         (T_PID_Handle *p_handle);
int32_t PID_getTargetValue(T_PID_Handle *p_handle);
int32_t PID_getMinValue   (T_PID_Handle *p_handle);
int32_t PID_getMaxValue   (T_PID_Handle *p_handle);
int32_t PID_getAntiWindUp (T_PID_Handle *p_handle);
int32_t PID_update        (T_PID_Handle *p_handle, int32_t p_currentValue, uint32_t p_timeDelta);
void    PID_logInfo       (T_PID_Handle *p_handle);

#endif /* __PID_H */
