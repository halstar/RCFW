#ifndef __WHEEL_H
#define __WHEEL_H

#include "stm32f1xx_hal.h"
#include "motor.h"
#include "encoder.h"
#include "pid.h"
#include "circular_buffer.h"

typedef struct T_WHL_Handle
{
  char         *name;
  T_MTR_Handle  motor;
  T_ENC_Handle  encoder;
  T_PID_Handle  pid;
  T_CBU_Handle  speedBuffer;
  float         averageSpeed;
  bool          isMotorOn;
} T_WHL_Handle;

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
              bool               p_isMotorOn);

void  WHL_turnMotorOn       (T_WHL_Handle *p_handle                             );
void  WHL_turnMotorOff      (T_WHL_Handle *p_handle                             );
void  WHL_updateEncoder     (T_WHL_Handle *p_handle, uint32_t        p_count    );
void  WHL_setDirection      (T_WHL_Handle *p_handle, T_MTR_DIRECTION p_direction);
void  WHL_setSpeed          (T_WHL_Handle *p_handle, uint32_t        p_speed    );
void  WHL_setPidKp          (T_WHL_Handle *p_handle, float           p_kp       );
void  WHL_setPidKi          (T_WHL_Handle *p_handle, float           p_ki       );
void  WHL_setPidKd          (T_WHL_Handle *p_handle, float           p_kd       );
void  WHL_setPidTarget      (T_WHL_Handle *p_handle, uint32_t        p_speed    );
void  WHL_setDirectTarget   (T_WHL_Handle *p_handle, uint32_t        p_speed    );
void  WHL_updateAverageSpeed(T_WHL_Handle *p_handle                             );
void  WHL_updatePidSpeed    (T_WHL_Handle *p_handle                             );
float WHL_getAverageSpeed   (T_WHL_Handle *p_handle                             );
void  WHL_logInfo           (T_WHL_Handle *p_handle                             );

#endif /* __WHEEL_H */
