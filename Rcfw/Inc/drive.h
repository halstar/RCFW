#ifndef __DRIVE_H
#define __DRIVE_H

#include <stdbool.h>

#include "stm32f1xx_hal.h"
#include "bluetooth_control.h"
#include "string_fifo.h"

typedef enum
{
  DRV_MODE_MANUAL_FIXED_SPEED = 0,
  DRV_MODE_MANUAL_VARIABLE_SPEED,
  DRV_MODE_MASTER_BOARD_CONTROL
} T_DRV_MODE;

void DRV_init(TIM_HandleTypeDef *p_pwmTimerHandle,
              TIM_HandleTypeDef *p_rearLeftEncoderTimerHandle,
              TIM_HandleTypeDef *p_rearRightEncoderTimerHandle,
              TIM_HandleTypeDef *p_frontRightEncoderTimerHandle,
              TIM_HandleTypeDef *p_frontLeftEncoderTimerHandle);

void       DRV_updateEncoder      (TIM_HandleTypeDef *p_encoderTimerHandle           );
void       DRV_updateAverageSpeeds(void                                              );
void       DRV_updateFromBluetooth(T_BLU_Data        *p_bluetoothData, bool p_logInfo);
void       DRV_updateFromCommands (T_SFO_Handle      *p_commandsFifo , bool p_logInfo);
void       DRV_reportVelocity     (void                                              );
T_DRV_MODE DRV_getMode            (void                                              );
void       DRV_logInfo            (bool               p_compactLog                   );

#endif /* __DRIVE_H */
