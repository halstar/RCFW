#ifndef __DRIVE_H
#define __DRIVE_H

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

void       DRV_updateEncoder      (TIM_HandleTypeDef *p_encoderTimerHandle                );
void       DRV_updateFromBluetooth(T_BLU_Data        *p_bluetoothData                     );
void       DRV_updateFromMaster   (T_SFO_Context     *p_commandsFifo, uint16_t p_deltaTime);
T_DRV_MODE DRV_getMode            (void                                                   );

#endif /* __DRIVE_H */
