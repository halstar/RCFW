#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "drive.h"

#include "pid.h"
#include "motor.h"
#include "encoder.h"
#include "chrono.h"
#include "wheel.h"
#include "main.h"
#include "log.h"
#include "const.h"
#include "setup.h"
#include "utils.h"
#include "circular_buffer.h"
#include "string_fifo.h"
#include "master_control.h"

typedef struct T_DRV_Context
{
  uint32_t     selectPressedStartTimeInS;
  bool         isBluetoothOn;
  bool         areMotorsOn;
  bool         isPidModeOn;
  T_DRV_MODE   mode;
  T_WHL_Handle wheelFrontRight;
  T_WHL_Handle wheelFrontLeft;
  T_WHL_Handle wheelRearLeft;
  T_WHL_Handle wheelRearRight;
} T_DRV_Context;

static T_DRV_Context g_DRV_context;

static void DRV_getSpeedFromCommand(char *p_string, uint32_t *p_speed);
static void DRV_getSpeedsFromCommand(char    *p_string,
                                     int32_t *p_frontRightSpeed,
                                     int32_t *p_frontLeftSpeed,
                                     int32_t *p_rearRightSpeed,
                                     int32_t *p_rearLeftSpeed);

static void DRV_toggleMotorsState          (void);
static void DRV_setDirectionsStop          (void);
static void DRV_setDirectionsForward       (void);
static void DRV_setDirectionsBackward      (void);
static void DRV_setDirectionsForwardRight  (void);
static void DRV_setDirectionsForwardLeft   (void);
static void DRV_setDirectionsBackwardRight (void);
static void DRV_setDirectionsBackwardLeft  (void);
static void DRV_setDirectionsTurnLeft      (void);
static void DRV_setDirectionsTurnRight     (void);
static void DRV_setDirectionsTranslateLeft (void);
static void DRV_setDirectionsTranslateRight(void);
static void DRV_setDirectionsCustom        (int32_t p_frontRightSpeed,
                                            int32_t p_frontLeftSpeed,
                                            int32_t p_rearRightSpeed,
                                            int32_t p_rearLeftSpeed);

static void DRV_stop             (void            );
static void DRV_moveForward      (uint32_t p_speed);
static void DRV_moveBackward     (uint32_t p_speed);
static void DRV_moveForwardRight (uint32_t p_speed);
static void DRV_moveForwardLeft  (uint32_t p_speed);
static void DRV_moveBackwardRight(uint32_t p_speed);
static void DRV_moveBackwardLeft (uint32_t p_speed);
static void DRV_turnLeft         (uint32_t p_speed);
static void DRV_turnRight        (uint32_t p_speed);
static void DRV_translateLeft    (uint32_t p_speed);
static void DRV_translateRight   (uint32_t p_speed);

void DRV_init(TIM_HandleTypeDef *p_pwmTimerHandle,
              TIM_HandleTypeDef *p_rearLeftEncoderTimerHandle,
              TIM_HandleTypeDef *p_rearRightEncoderTimerHandle,
              TIM_HandleTypeDef *p_frontRightEncoderTimerHandle,
              TIM_HandleTypeDef *p_frontLeftEncoderTimerHandle)
{
  LOG_info("Initializing Drive module");

  g_DRV_context.selectPressedStartTimeInS = 0;

  /* Setup all 4 wheels */
  WHL_init(&g_DRV_context.wheelFrontRight,
            CST_FRONT_RIGHT_WHEEL_NAME,
            MOTOR_FRONT_RIGHT_OUT_1_GPIO_Port,
            MOTOR_FRONT_RIGHT_OUT_1_Pin,
            MOTOR_FRONT_RIGHT_OUT_2_GPIO_Port,
            MOTOR_FRONT_RIGHT_OUT_2_Pin,
            p_pwmTimerHandle,
            TIM_CHANNEL_4,
            true,
            p_frontRightEncoderTimerHandle,
            STP_DEFAULT_MOTORS_MODE);

  WHL_init(&g_DRV_context.wheelFrontLeft,
            CST_FRONT_LEFT_WHEEL_NAME,
            MOTOR_FRONT_LEFT_OUT_1_GPIO_Port,
            MOTOR_FRONT_LEFT_OUT_1_Pin,
            MOTOR_FRONT_LEFT_OUT_2_GPIO_Port,
            MOTOR_FRONT_LEFT_OUT_2_Pin,
            p_pwmTimerHandle,
            TIM_CHANNEL_3,
            false,
            p_frontLeftEncoderTimerHandle,
            STP_DEFAULT_MOTORS_MODE);

  WHL_init(&g_DRV_context.wheelRearLeft,
            CST_REAR_LEFT_WHEEL_NAME,
            MOTOR_REAR_LEFT_OUT_1_GPIO_Port,
            MOTOR_REAR_LEFT_OUT_1_Pin,
            MOTOR_REAR_LEFT_OUT_2_GPIO_Port,
            MOTOR_REAR_LEFT_OUT_2_Pin,
            p_pwmTimerHandle,
            TIM_CHANNEL_2,
            false,
            p_rearLeftEncoderTimerHandle,
            STP_DEFAULT_MOTORS_MODE);

  WHL_init(&g_DRV_context.wheelRearRight,
            CST_REAR_RIGHT_WHEEL_NAME,
            MOTOR_REAR_RIGHT_OUT_1_GPIO_Port,
            MOTOR_REAR_RIGHT_OUT_1_Pin,
            MOTOR_REAR_RIGHT_OUT_2_GPIO_Port,
            MOTOR_REAR_RIGHT_OUT_2_Pin,
            p_pwmTimerHandle,
            TIM_CHANNEL_1,
            true,
            p_rearRightEncoderTimerHandle,
            STP_DEFAULT_MOTORS_MODE);

  /* Consider that bluetooth controller is OFF by default. Pressing START is needed. */
  g_DRV_context.isBluetoothOn = false;

  /* Activate motors or not by default (de-activating them is used for debug) */
  g_DRV_context.areMotorsOn = STP_DEFAULT_MOTORS_MODE;

  /* Use wheels PIDs by default */
  g_DRV_context.isPidModeOn = true;

  /* Start with default drive mode (different in debug and in release) */
  g_DRV_context.mode = STP_DEFAULT_DRIVE_MODE;

  DRV_logInfo(false);

  return;
}

void DRV_updateEncoder(TIM_HandleTypeDef *p_encoderTimerHandle)
{
  int16_t l_count;

  l_count = __HAL_TIM_GET_COUNTER(p_encoderTimerHandle);

  /* Check the handle of the triggering timer and update encoder accordingly */
  if (p_encoderTimerHandle == g_DRV_context.wheelFrontRight.encoder.timerHandle)
  {
    WHL_updateEncoder(&g_DRV_context.wheelFrontRight, l_count);
  }
  else if (p_encoderTimerHandle == g_DRV_context.wheelFrontLeft.encoder.timerHandle)
  {
    WHL_updateEncoder(&g_DRV_context.wheelFrontLeft, l_count);
  }
  else if (p_encoderTimerHandle == g_DRV_context.wheelRearRight.encoder.timerHandle)
  {
    WHL_updateEncoder(&g_DRV_context.wheelRearRight, l_count);
  }
  else if (p_encoderTimerHandle == g_DRV_context.wheelRearLeft.encoder.timerHandle)
  {
    WHL_updateEncoder(&g_DRV_context.wheelRearLeft, l_count);
  }
  else
  {
    ; /* Nothing to do */
  }

  return;
}

void DRV_updateAverageSpeeds(void)
{
  /* Update all 4 wheels average speeds */
  WHL_updateAverageSpeed(&g_DRV_context.wheelFrontRight);
  WHL_updateAverageSpeed(&g_DRV_context.wheelFrontLeft );
  WHL_updateAverageSpeed(&g_DRV_context.wheelRearRight );
  WHL_updateAverageSpeed(&g_DRV_context.wheelRearLeft  );

  return;
}

void DRV_updateFromBluetooth(T_BLU_Data *p_bluetoothData, bool p_logInfo)
{
  uint32_t l_currentTimeInS;
  uint32_t l_speed;

  if (p_bluetoothData->button == BLU_BUTTON_START)
  {
    if (g_DRV_context.isBluetoothOn == false)
    {
      LOG_info("Bluetooth controller detected ON");
      g_DRV_context.isBluetoothOn = true;
    }
    else
    {
      ; /* Nothing to do */
    }
  }
  else if (g_DRV_context.isBluetoothOn == false)
  {
    ; /* Nothing to do */
  }
  else
  {
    l_currentTimeInS = UTI_getTimeS();

    /* Check possible requested mode change */
    switch (p_bluetoothData->button)
    {
      case BLU_BUTTON_PINK_SQUARE:
        if (g_DRV_context.mode != DRV_MODE_MANUAL_FIXED_SPEED)
        {
          DRV_stop();
          g_DRV_context.mode = DRV_MODE_MANUAL_FIXED_SPEED;
          LOG_info("Drive mode now DRV_MODE_MANUAL_FIXED_SPEED");
        }
        else
        {
          ; /* Nothing to do */
        }
        break;

      case BLU_BUTTON_BLUE_CROSS:
        if (g_DRV_context.mode != DRV_MODE_MANUAL_VARIABLE_SPEED)
        {
          DRV_stop();
          g_DRV_context.mode = DRV_MODE_MANUAL_VARIABLE_SPEED;
          LOG_info("Drive mode now DRV_MODE_MANUAL_VARIABLE_SPEED");
        }
        else
        {
          ; /* Nothing to do */
        }
        break;

      case BLU_BUTTON_RED_CIRCLE:
        if (g_DRV_context.mode != DRV_MODE_MASTER_BOARD_CONTROL)
        {
          DRV_stop();
          g_DRV_context.mode = DRV_MODE_MASTER_BOARD_CONTROL;
          LOG_info("Drive mode now DRV_MODE_MASTER_BOARD_CONTROL");
        }
        else
        {
          ; /* Nothing to do */
        }
        break;

      case BLU_BUTTON_SELECT:
        if (g_DRV_context.selectPressedStartTimeInS == 0)
        {
          g_DRV_context.selectPressedStartTimeInS = l_currentTimeInS;

          DRV_toggleMotorsState();
        }
        else if (l_currentTimeInS - g_DRV_context.selectPressedStartTimeInS < STP_BUTTONS_DEBOUNCE_PERIOD_IN_S)
        {
          ; /* Nothing to do */
        }
        else
        {
          g_DRV_context.selectPressedStartTimeInS = 0;
        }
        break;

      case BLU_BUTTON_NONE:
      default:
        ; /* Nothing to do */
        break;
    }

    /* Master board control mode is an automated mode, so that we will */
    /* ignore any direction/button press we received via bluetooth.    */
    if (g_DRV_context.mode == DRV_MODE_MASTER_BOARD_CONTROL)
    {
      ; /* Nothing to do */
    }
    /* Manual mode, applying directions received by bluetooth */
    else
    {
      if (p_bluetoothData->leftY > STP_JOYSTICKS_THRESHOLD)
      {
        l_speed = g_DRV_context.mode == DRV_MODE_MANUAL_FIXED_SPEED ? STP_JOYSTICKS_FIXED_SPEED : p_bluetoothData->leftY;

        DRV_moveForward(l_speed);
      }
      else if (p_bluetoothData->rightY > STP_JOYSTICKS_THRESHOLD)
      {
        l_speed = g_DRV_context.mode == DRV_MODE_MANUAL_FIXED_SPEED ? STP_JOYSTICKS_FIXED_SPEED : p_bluetoothData->rightY;

        DRV_moveForward(l_speed);
      }
      else if (p_bluetoothData->leftY < -STP_JOYSTICKS_THRESHOLD)
      {
        l_speed = g_DRV_context.mode == DRV_MODE_MANUAL_FIXED_SPEED ? STP_JOYSTICKS_FIXED_SPEED : -p_bluetoothData->leftY;

        DRV_moveBackward(l_speed);
      }
      else if (p_bluetoothData->rightY < -STP_JOYSTICKS_THRESHOLD)
      {
        l_speed = g_DRV_context.mode == DRV_MODE_MANUAL_FIXED_SPEED ? STP_JOYSTICKS_FIXED_SPEED : -p_bluetoothData->rightY;

        DRV_moveBackward(l_speed);
      }
      else if (p_bluetoothData->leftX < -STP_JOYSTICKS_THRESHOLD)
      {
        l_speed = g_DRV_context.mode == DRV_MODE_MANUAL_FIXED_SPEED ? STP_JOYSTICKS_FIXED_SPEED : -p_bluetoothData->leftX;

        DRV_turnLeft(l_speed);
      }
      else if (p_bluetoothData->leftX > STP_JOYSTICKS_THRESHOLD)
      {
        l_speed = g_DRV_context.mode == DRV_MODE_MANUAL_FIXED_SPEED ? STP_JOYSTICKS_FIXED_SPEED : p_bluetoothData->leftX;

        DRV_turnRight(l_speed);
      }
      else if (p_bluetoothData->rightX < -STP_JOYSTICKS_THRESHOLD)
      {
        l_speed = g_DRV_context.mode == DRV_MODE_MANUAL_FIXED_SPEED ? STP_JOYSTICKS_FIXED_SPEED : -p_bluetoothData->rightX;

        DRV_translateLeft(l_speed);
      }
      else if (p_bluetoothData->rightX > STP_JOYSTICKS_THRESHOLD)
      {
        l_speed = g_DRV_context.mode == DRV_MODE_MANUAL_FIXED_SPEED ? STP_JOYSTICKS_FIXED_SPEED : p_bluetoothData->rightX;

        DRV_translateRight(l_speed);
      }
      else if (p_bluetoothData->button == BLU_BUTTON_L1)
      {
        DRV_moveForwardLeft(STP_BUTTONS_FIXED_SPEED);
      }
      else if (p_bluetoothData->button == BLU_BUTTON_L2)
      {
        DRV_moveBackwardRight(STP_BUTTONS_FIXED_SPEED);
      }
      else if (p_bluetoothData->button == BLU_BUTTON_R1)
      {
        DRV_moveForwardRight(STP_BUTTONS_FIXED_SPEED);
      }
      else if (p_bluetoothData->button == BLU_BUTTON_R2)
      {
        DRV_moveBackwardLeft(STP_BUTTONS_FIXED_SPEED);
      }
      else
      {
        /* Most of the time, we will get here */
        DRV_stop();
      }

      if (p_logInfo == true)
      {
        DRV_logInfo(true);
      }
      else
      {
        ; /* Nothing to do */
      }
    }
  }

  return;
}

void DRV_updateFromCommands(T_SFO_Handle *p_commandsFifo, bool p_logInfo)
{
  T_SFO_data l_command;
  uint32_t   l_speed;
  int32_t    l_frontRightSpeed;
  int32_t    l_frontLeftSpeed;
  int32_t    l_rearRightSpeed;
  int32_t    l_rearLeftSpeed;
  float      l_value;

  /* Ignore master board data when a manual mode is selected */
  if (g_DRV_context.mode != DRV_MODE_MASTER_BOARD_CONTROL)
  {
    ; /* Nothing to do */
  }
  else
  {
    /* Deal with only one command per cycle. If a FIFO overflow occurs, */
    /* FIFO string will report an error, showing us a design issue...   */
    if (SFO_getCount(p_commandsFifo) != 0)
    {
      SFO_logInfo(p_commandsFifo);
      SFO_pop    (p_commandsFifo         , &l_command);
      LOG_info   ("Drive got command: %s",  l_command);

      /* Most commands expect PID mode */
      g_DRV_context.isPidModeOn = true;

      /* Stop */
      if ((l_command[0] == 'S') && (l_command[1] == 'T'))
      {
        DRV_setDirectionsStop();

        WHL_setPidTarget(&g_DRV_context.wheelFrontRight, 0);
        WHL_setPidTarget(&g_DRV_context.wheelFrontLeft , 0);
        WHL_setPidTarget(&g_DRV_context.wheelRearRight , 0);
        WHL_setPidTarget(&g_DRV_context.wheelRearLeft  , 0);
      }
      /* Forward Straight */
      else if ((l_command[0] == 'F') && (l_command[1] == 'S'))
      {
        DRV_getSpeedFromCommand(&l_command[2], &l_speed);

        DRV_setDirectionsForward();

        WHL_setPidTarget(&g_DRV_context.wheelFrontRight, l_speed);
        WHL_setPidTarget(&g_DRV_context.wheelFrontLeft , l_speed);
        WHL_setPidTarget(&g_DRV_context.wheelRearRight , l_speed);
        WHL_setPidTarget(&g_DRV_context.wheelRearLeft  , l_speed);
      }
      /* Move Backward */
      else if ((l_command[0] == 'B') && (l_command[1] == 'S'))
      {
        DRV_getSpeedFromCommand(&l_command[2], &l_speed);

        DRV_setDirectionsBackward();

        WHL_setPidTarget(&g_DRV_context.wheelFrontRight, l_speed);
        WHL_setPidTarget(&g_DRV_context.wheelFrontLeft , l_speed);
        WHL_setPidTarget(&g_DRV_context.wheelRearRight , l_speed);
        WHL_setPidTarget(&g_DRV_context.wheelRearLeft  , l_speed);
      }
      /* TuRn (i.e. Rotate) Left */
      else if ((l_command[0] == 'R') && (l_command[1] == 'L'))
      {
        DRV_getSpeedFromCommand(&l_command[2], &l_speed);

        DRV_setDirectionsTurnLeft();

        WHL_setPidTarget(&g_DRV_context.wheelFrontRight, l_speed);
        WHL_setPidTarget(&g_DRV_context.wheelFrontLeft , l_speed);
        WHL_setPidTarget(&g_DRV_context.wheelRearRight , l_speed);
        WHL_setPidTarget(&g_DRV_context.wheelRearLeft  , l_speed);
      }
      /* TuRn (i.e. Rotate) Right */
      else if ((l_command[0] == 'R') && (l_command[1] == 'R'))
      {
        DRV_getSpeedFromCommand(&l_command[2], &l_speed);

        DRV_setDirectionsTurnRight();

        WHL_setPidTarget(&g_DRV_context.wheelFrontRight, l_speed);
        WHL_setPidTarget(&g_DRV_context.wheelFrontLeft , l_speed);
        WHL_setPidTarget(&g_DRV_context.wheelRearRight , l_speed);
        WHL_setPidTarget(&g_DRV_context.wheelRearLeft  , l_speed);
      }
      /* Translate Left */
      else if ((l_command[0] == 'T') && (l_command[1] == 'L'))
      {
        DRV_getSpeedFromCommand(&l_command[2], &l_speed);

        DRV_setDirectionsTranslateLeft();

        WHL_setPidTarget(&g_DRV_context.wheelFrontRight, l_speed);
        WHL_setPidTarget(&g_DRV_context.wheelFrontLeft , l_speed);
        WHL_setPidTarget(&g_DRV_context.wheelRearRight , l_speed);
        WHL_setPidTarget(&g_DRV_context.wheelRearLeft  , l_speed);
      }
      /* Translate Right */
      else if ((l_command[0] == 'T') && (l_command[1] == 'R'))
      {
        DRV_getSpeedFromCommand(&l_command[2], &l_speed);

        DRV_setDirectionsTranslateRight();

        WHL_setPidTarget(&g_DRV_context.wheelFrontRight, l_speed);
        WHL_setPidTarget(&g_DRV_context.wheelFrontLeft , l_speed);
        WHL_setPidTarget(&g_DRV_context.wheelRearRight , l_speed);
        WHL_setPidTarget(&g_DRV_context.wheelRearLeft  , l_speed);
      }
      /* Forward Left */
      else if ((l_command[0] == 'F') && (l_command[1] == 'L'))
      {
        DRV_getSpeedFromCommand(&l_command[2], &l_speed);

        DRV_setDirectionsForwardLeft();

        WHL_setPidTarget(&g_DRV_context.wheelFrontRight, l_speed);
        WHL_setPidTarget(&g_DRV_context.wheelFrontLeft ,       0);
        WHL_setPidTarget(&g_DRV_context.wheelRearRight ,       0);
        WHL_setPidTarget(&g_DRV_context.wheelRearLeft  , l_speed);
      }
      /* Forward Right */
      else if ((l_command[0] == 'F') && (l_command[1] == 'R'))
      {
        DRV_getSpeedFromCommand(&l_command[2], &l_speed);

        DRV_setDirectionsForwardRight();

        WHL_setPidTarget(&g_DRV_context.wheelFrontRight,       0);
        WHL_setPidTarget(&g_DRV_context.wheelFrontLeft , l_speed);
        WHL_setPidTarget(&g_DRV_context.wheelRearRight , l_speed);
        WHL_setPidTarget(&g_DRV_context.wheelRearLeft  ,       0);
      }
      /* Backward Left */
      else if ((l_command[0] == 'B') && (l_command[1] == 'L'))
      {
        DRV_getSpeedFromCommand(&l_command[2], &l_speed);

        DRV_setDirectionsBackwardLeft();

        WHL_setPidTarget(&g_DRV_context.wheelFrontRight,       0);
        WHL_setPidTarget(&g_DRV_context.wheelFrontLeft , l_speed);
        WHL_setPidTarget(&g_DRV_context.wheelRearRight , l_speed);
        WHL_setPidTarget(&g_DRV_context.wheelRearLeft  ,       0);
      }
      /* Backward Right */
      else if ((l_command[0] == 'B') && (l_command[1] == 'R'))
      {
        DRV_getSpeedFromCommand(&l_command[2], &l_speed);

        DRV_setDirectionsBackwardRight();

        WHL_setPidTarget(&g_DRV_context.wheelFrontRight, l_speed);
        WHL_setPidTarget(&g_DRV_context.wheelFrontLeft ,       0);
        WHL_setPidTarget(&g_DRV_context.wheelRearRight ,       0);
        WHL_setPidTarget(&g_DRV_context.wheelRearLeft  , l_speed);
      }
      /* Custom */
      else if (l_command[0] == 'C')
      {
        g_DRV_context.isPidModeOn = false;

        DRV_getSpeedsFromCommand(&l_command[1],
                                 &l_frontRightSpeed,
                                 &l_frontLeftSpeed,
                                 &l_rearRightSpeed,
                                 &l_rearLeftSpeed);

        DRV_setDirectionsCustom(l_frontRightSpeed,
                                l_frontLeftSpeed,
                                l_rearRightSpeed,
                                l_rearLeftSpeed);

        WHL_setDirectTarget(&g_DRV_context.wheelFrontRight, abs(l_frontRightSpeed));
        WHL_setDirectTarget(&g_DRV_context.wheelFrontLeft , abs(l_frontLeftSpeed ));
        WHL_setDirectTarget(&g_DRV_context.wheelRearRight , abs(l_rearRightSpeed ));
        WHL_setDirectTarget(&g_DRV_context.wheelRearLeft  , abs(l_rearLeftSpeed  ));
      }
      else if ((l_command[0] == 'K') && (l_command[1] == 'P'))
      {
        l_value = atof(&l_command[2]);

        WHL_setPidKp(&g_DRV_context.wheelFrontRight, l_value);
        WHL_setPidKp(&g_DRV_context.wheelFrontLeft , l_value);
        WHL_setPidKp(&g_DRV_context.wheelRearRight , l_value);
        WHL_setPidKp(&g_DRV_context.wheelRearLeft  , l_value);
      }
      else if ((l_command[0] == 'K') && (l_command[1] == 'I'))
      {
        l_value = atof(&l_command[2]);

        WHL_setPidKi(&g_DRV_context.wheelFrontRight, l_value);
        WHL_setPidKi(&g_DRV_context.wheelFrontLeft , l_value);
        WHL_setPidKi(&g_DRV_context.wheelRearRight , l_value);
        WHL_setPidKi(&g_DRV_context.wheelRearLeft  , l_value);
      }
      else if ((l_command[0] == 'K') && (l_command[1] == 'D'))
      {
        l_value = atof(&l_command[2]);

        WHL_setPidKd(&g_DRV_context.wheelFrontRight, l_value);
        WHL_setPidKd(&g_DRV_context.wheelFrontLeft , l_value);
        WHL_setPidKd(&g_DRV_context.wheelRearRight , l_value);
        WHL_setPidKd(&g_DRV_context.wheelRearLeft  , l_value);
      }
      else
      {
        LOG_error("Drive got unsupported command: '%s'", l_command);
      }
    }

    if (g_DRV_context.isPidModeOn == true)
    {
      /* Update all 4 wheels PIDs, adjusting speeds, to reach targets */
      WHL_updatePidSpeed(&g_DRV_context.wheelFrontRight);
      WHL_updatePidSpeed(&g_DRV_context.wheelFrontLeft );
      WHL_updatePidSpeed(&g_DRV_context.wheelRearRight );
      WHL_updatePidSpeed(&g_DRV_context.wheelRearLeft  );
    }
    else
    {
      /* Nothing to do */
    }

    if (p_logInfo == true)
    {
      DRV_logInfo(true);
    }
    else
    {
      ; /* Nothing to do */
    }
  }

  return;
}

void DRV_reportVelocity(void)
{
  float l_averageSpeedFrontRight;
  float l_averageSpeedFrontLeft;
  float l_averageSpeedRearRight;
  float l_averageSpeedRearLeft;
  char  l_buffer[CST_MASTER_VELOCITY_STRING_LENGTH];

  (void)memset((void *)l_buffer, 0, CST_MASTER_VELOCITY_STRING_LENGTH);

  l_averageSpeedFrontRight = WHL_getAverageSpeed(&g_DRV_context.wheelFrontRight);
  l_averageSpeedFrontLeft  = WHL_getAverageSpeed(&g_DRV_context.wheelFrontLeft );
  l_averageSpeedRearRight  = WHL_getAverageSpeed(&g_DRV_context.wheelRearRight );
  l_averageSpeedRearLeft   = WHL_getAverageSpeed(&g_DRV_context.wheelRearLeft  );

  (void)snprintf(l_buffer,
                 CST_MASTER_VELOCITY_STRING_LENGTH,
                 "S%2d %2d %2d %2d\r",
            (int)l_averageSpeedFrontRight,
            (int)l_averageSpeedFrontLeft ,
            (int)l_averageSpeedRearRight ,
            (int)l_averageSpeedRearLeft  );

  MAS_sendString(l_buffer, strnlen(l_buffer, CST_MASTER_VELOCITY_STRING_LENGTH));

  return;
}

T_DRV_MODE DRV_getMode(void)
{
  return g_DRV_context.mode;
}

void DRV_logInfo(bool p_compactLog)
{
  if (g_DRV_context.mode == DRV_MODE_MANUAL_FIXED_SPEED)
  {
    LOG_info("Drive mode  : MANUAL FIXED SPEED");
  }
  else if (g_DRV_context.mode == DRV_MODE_MANUAL_VARIABLE_SPEED)
  {
    LOG_info("Drive mode  : MANUAL VARIABLE SPEED");
  }
  else if (g_DRV_context.mode == DRV_MODE_MASTER_BOARD_CONTROL)
  {
    LOG_info("Drive mode  : MASTER BOARD CONTROL");
  }
  else
  {
    LOG_error("Unsupported drive mode: %u", g_DRV_context.mode);
  }

  if (g_DRV_context.areMotorsOn == true)
  {
    LOG_info("Drive motors: ON");
  }
  else
  {
    LOG_info("Drive motors: OFF");
  }

  WHL_logInfo(&g_DRV_context.wheelFrontRight);
  WHL_logInfo(&g_DRV_context.wheelFrontLeft );
  WHL_logInfo(&g_DRV_context.wheelRearRight );
  WHL_logInfo(&g_DRV_context.wheelRearLeft  );

  return;
}

static void DRV_getSpeedFromCommand(char *p_string, uint32_t *p_speed)
{
  uint32_t l_speed;

  l_speed = atoi(p_string);

  /* Check that speed is in allowed range */
  if ((l_speed < STP_MASTER_MIN_SPEED) || (l_speed > STP_MASTER_MAX_SPEED))
  {
    LOG_error("Drive got out of range speed: %u", l_speed);
  }
  else
  {
    *p_speed = UTI_normalizeIntValue(l_speed ,
                                     STP_MASTER_MIN_SPEED,
                                     STP_MASTER_MAX_SPEED,
                                     STP_DRIVE_MIN_SPEED,
                                     STP_DRIVE_MAX_SPEED,
                                     false);
  }

  return;
}


static void DRV_getSpeedsFromCommand(char    *p_string,
                                     int32_t *p_frontRightSpeed,
                                     int32_t *p_frontLeftSpeed,
                                     int32_t *p_rearRightSpeed,
                                     int32_t *p_rearLeftSpeed)
{
  int32_t l_frontRightSpeed = 0;
  int32_t l_frontLeftSpeed  = 0;
  int32_t l_rearRightSpeed  = 0;
  int32_t l_rearLeftSpeed   = 0;

  *p_frontRightSpeed = 0;
  *p_frontLeftSpeed  = 0;
  *p_rearRightSpeed  = 0;
  *p_rearLeftSpeed   = 0;

  (void)sscanf(p_string,
               "%ld %ld %ld %ld",
               &l_frontRightSpeed,
               &l_frontLeftSpeed,
               &l_rearRightSpeed,
               &l_rearLeftSpeed);

  /* Make sure that all speed is in allowed range */
  l_frontRightSpeed = UTI_clampIntValue(l_frontRightSpeed, -STP_MASTER_MAX_SPEED, STP_MASTER_MAX_SPEED, true, 0);
  l_frontLeftSpeed  = UTI_clampIntValue(l_frontLeftSpeed , -STP_MASTER_MAX_SPEED, STP_MASTER_MAX_SPEED, true, 0);
  l_rearRightSpeed  = UTI_clampIntValue(l_rearRightSpeed , -STP_MASTER_MAX_SPEED, STP_MASTER_MAX_SPEED, true, 0);
  l_rearLeftSpeed   = UTI_clampIntValue(l_rearLeftSpeed  , -STP_MASTER_MAX_SPEED, STP_MASTER_MAX_SPEED, true, 0);

  if ((l_frontRightSpeed < -STP_MASTER_MAX_SPEED) || (l_frontRightSpeed > STP_MASTER_MAX_SPEED)
   || (l_frontLeftSpeed  < -STP_MASTER_MAX_SPEED) || (l_frontLeftSpeed  > STP_MASTER_MAX_SPEED)
   || (l_rearRightSpeed  < -STP_MASTER_MAX_SPEED) || (l_rearRightSpeed  > STP_MASTER_MAX_SPEED)
   || (l_rearLeftSpeed   < -STP_MASTER_MAX_SPEED) || (l_rearLeftSpeed   > STP_MASTER_MAX_SPEED))
  {
    LOG_error("Got out of range speed(s) / clamped speed(s): %d %d %d %d",
              l_frontRightSpeed,
              l_frontLeftSpeed,
              l_rearRightSpeed,
              l_rearLeftSpeed);
  }
  else
  {
    /* Nothing to do */
  }

  *p_frontRightSpeed = UTI_normalizeIntValue(abs(l_frontRightSpeed),
                                             STP_MASTER_MIN_SPEED,
                                             STP_MASTER_MAX_SPEED,
                                             STP_DRIVE_MIN_SPEED,
                                             STP_DRIVE_MAX_SPEED,
                                             l_frontRightSpeed > 0 ? false : true);

  *p_frontLeftSpeed  = UTI_normalizeIntValue(abs(l_frontLeftSpeed),
                                             STP_MASTER_MIN_SPEED,
                                             STP_MASTER_MAX_SPEED,
                                             STP_DRIVE_MIN_SPEED,
                                             STP_DRIVE_MAX_SPEED,
                                             l_frontLeftSpeed > 0 ? false : true);

  *p_rearRightSpeed  = UTI_normalizeIntValue(abs(l_rearRightSpeed),
                                             STP_MASTER_MIN_SPEED,
                                             STP_MASTER_MAX_SPEED,
                                             STP_DRIVE_MIN_SPEED,
                                             STP_DRIVE_MAX_SPEED,
                                             l_rearRightSpeed > 0 ? false : true);

  *p_rearLeftSpeed   = UTI_normalizeIntValue(abs(l_rearLeftSpeed),
                                             STP_MASTER_MIN_SPEED,
                                             STP_MASTER_MAX_SPEED,
                                             STP_DRIVE_MIN_SPEED,
                                             STP_DRIVE_MAX_SPEED,
                                             l_rearLeftSpeed > 0 ? false : true);

  return;
}

static void DRV_toggleMotorsState(void)
{
  if (g_DRV_context.areMotorsOn == false)
  {
    LOG_info("Drive turning motor ON");

    WHL_turnMotorOn(&g_DRV_context.wheelFrontRight);
    WHL_turnMotorOn(&g_DRV_context.wheelFrontLeft );
    WHL_turnMotorOn(&g_DRV_context.wheelRearRight );
    WHL_turnMotorOn(&g_DRV_context.wheelRearLeft  );

    g_DRV_context.areMotorsOn = true;
  }
  else
  {
    LOG_info("Drive turning motor OFF");

    WHL_turnMotorOff(&g_DRV_context.wheelFrontRight);
    WHL_turnMotorOff(&g_DRV_context.wheelFrontLeft );
    WHL_turnMotorOff(&g_DRV_context.wheelRearRight );
    WHL_turnMotorOff(&g_DRV_context.wheelRearLeft  );

    g_DRV_context.areMotorsOn = false;
  }

  return;
}

static void DRV_setDirectionsStop(void)
{
  WHL_setDirection(&g_DRV_context.wheelFrontRight, MTR_DIRECTION_STOP);
  WHL_setDirection(&g_DRV_context.wheelFrontLeft , MTR_DIRECTION_STOP);
  WHL_setDirection(&g_DRV_context.wheelRearRight , MTR_DIRECTION_STOP);
  WHL_setDirection(&g_DRV_context.wheelRearLeft  , MTR_DIRECTION_STOP);

  return;
}

static void DRV_setDirectionsForward(void)
{
  WHL_setDirection(&g_DRV_context.wheelFrontRight, MTR_DIRECTION_FORWARD);
  WHL_setDirection(&g_DRV_context.wheelFrontLeft , MTR_DIRECTION_FORWARD);
  WHL_setDirection(&g_DRV_context.wheelRearRight , MTR_DIRECTION_FORWARD);
  WHL_setDirection(&g_DRV_context.wheelRearLeft  , MTR_DIRECTION_FORWARD);

  return;
}

static void DRV_setDirectionsBackward(void)
{
  WHL_setDirection(&g_DRV_context.wheelFrontRight, MTR_DIRECTION_BACKWARD);
  WHL_setDirection(&g_DRV_context.wheelFrontLeft , MTR_DIRECTION_BACKWARD);
  WHL_setDirection(&g_DRV_context.wheelRearRight , MTR_DIRECTION_BACKWARD);
  WHL_setDirection(&g_DRV_context.wheelRearLeft  , MTR_DIRECTION_BACKWARD);

  return;
}

static void DRV_setDirectionsForwardRight(void)
{
  WHL_setDirection(&g_DRV_context.wheelFrontLeft, MTR_DIRECTION_FORWARD);
  WHL_setDirection(&g_DRV_context.wheelRearRight, MTR_DIRECTION_FORWARD);

  return;
}

static void DRV_setDirectionsForwardLeft(void)
{
  WHL_setDirection(&g_DRV_context.wheelFrontRight, MTR_DIRECTION_FORWARD);
  WHL_setDirection(&g_DRV_context.wheelRearLeft  , MTR_DIRECTION_FORWARD);

  return;
}

static void DRV_setDirectionsBackwardRight(void)
{
  WHL_setDirection(&g_DRV_context.wheelFrontRight, MTR_DIRECTION_BACKWARD);
  WHL_setDirection(&g_DRV_context.wheelRearLeft  , MTR_DIRECTION_BACKWARD);

  return;
}

static void DRV_setDirectionsBackwardLeft(void)
{
  WHL_setDirection(&g_DRV_context.wheelFrontLeft, MTR_DIRECTION_BACKWARD);
  WHL_setDirection(&g_DRV_context.wheelRearRight, MTR_DIRECTION_BACKWARD);

  return;
}

static void DRV_setDirectionsTurnLeft(void)
{
  WHL_setDirection(&g_DRV_context.wheelFrontRight, MTR_DIRECTION_FORWARD );
  WHL_setDirection(&g_DRV_context.wheelFrontLeft , MTR_DIRECTION_BACKWARD);
  WHL_setDirection(&g_DRV_context.wheelRearRight , MTR_DIRECTION_FORWARD );
  WHL_setDirection(&g_DRV_context.wheelRearLeft  , MTR_DIRECTION_BACKWARD);

  return;
}

static void DRV_setDirectionsTurnRight(void)
{
  WHL_setDirection(&g_DRV_context.wheelFrontRight, MTR_DIRECTION_BACKWARD);
  WHL_setDirection(&g_DRV_context.wheelFrontLeft , MTR_DIRECTION_FORWARD );
  WHL_setDirection(&g_DRV_context.wheelRearRight , MTR_DIRECTION_BACKWARD);
  WHL_setDirection(&g_DRV_context.wheelRearLeft  , MTR_DIRECTION_FORWARD );

  return;
}

static void DRV_setDirectionsTranslateLeft(void)
{
  WHL_setDirection(&g_DRV_context.wheelFrontRight, MTR_DIRECTION_FORWARD );
  WHL_setDirection(&g_DRV_context.wheelFrontLeft , MTR_DIRECTION_BACKWARD);
  WHL_setDirection(&g_DRV_context.wheelRearRight , MTR_DIRECTION_BACKWARD);
  WHL_setDirection(&g_DRV_context.wheelRearLeft  , MTR_DIRECTION_FORWARD );

  return;
}

static void DRV_setDirectionsTranslateRight(void)
{
  WHL_setDirection(&g_DRV_context.wheelFrontRight, MTR_DIRECTION_BACKWARD);
  WHL_setDirection(&g_DRV_context.wheelFrontLeft , MTR_DIRECTION_FORWARD );
  WHL_setDirection(&g_DRV_context.wheelRearRight , MTR_DIRECTION_FORWARD );
  WHL_setDirection(&g_DRV_context.wheelRearLeft  , MTR_DIRECTION_BACKWARD);

  return;
}

static void DRV_setDirectionsCustom(int32_t p_frontRightSpeed,
                                    int32_t p_frontLeftSpeed,
                                    int32_t p_rearRightSpeed,
                                    int32_t p_rearLeftSpeed)
{
  if (p_frontRightSpeed >= 0)
  {
    WHL_setDirection(&g_DRV_context.wheelFrontRight, MTR_DIRECTION_FORWARD);
  }
  else
  {
    WHL_setDirection(&g_DRV_context.wheelFrontRight, MTR_DIRECTION_BACKWARD);
  }

  if (p_frontLeftSpeed >= 0)
  {
    WHL_setDirection(&g_DRV_context.wheelFrontLeft, MTR_DIRECTION_FORWARD);
  }
  else
  {
    WHL_setDirection(&g_DRV_context.wheelFrontLeft, MTR_DIRECTION_BACKWARD);
  }

  if (p_rearRightSpeed >= 0)
  {
    WHL_setDirection(&g_DRV_context.wheelRearRight, MTR_DIRECTION_FORWARD);
  }
  else
  {
    WHL_setDirection(&g_DRV_context.wheelRearRight, MTR_DIRECTION_BACKWARD);
  }

  if (p_rearLeftSpeed >= 0)
  {
    WHL_setDirection(&g_DRV_context.wheelRearLeft, MTR_DIRECTION_FORWARD);
  }
  else
  {
    WHL_setDirection(&g_DRV_context.wheelRearLeft, MTR_DIRECTION_BACKWARD);
  }

  return;
}


static void DRV_stop(void)
{
  DRV_setDirectionsStop();

  WHL_setSpeed(&g_DRV_context.wheelFrontRight, 0);
  WHL_setSpeed(&g_DRV_context.wheelFrontLeft , 0);
  WHL_setSpeed(&g_DRV_context.wheelRearRight , 0);
  WHL_setSpeed(&g_DRV_context.wheelRearLeft  , 0);

  return;
}

static void DRV_moveForward(uint32_t p_speed)
{
  LOG_debug("Moving forward @%u", p_speed);

  DRV_setDirectionsForward();

  WHL_setSpeed(&g_DRV_context.wheelFrontRight, p_speed);
  WHL_setSpeed(&g_DRV_context.wheelFrontLeft , p_speed);
  WHL_setSpeed(&g_DRV_context.wheelRearRight , p_speed);
  WHL_setSpeed(&g_DRV_context.wheelRearLeft  , p_speed);

  return;
}

static void DRV_moveBackward(uint32_t p_speed)
{
  LOG_debug("Moving backward @%u", p_speed);

  DRV_setDirectionsBackward();

  WHL_setSpeed(&g_DRV_context.wheelFrontRight, p_speed);
  WHL_setSpeed(&g_DRV_context.wheelFrontLeft , p_speed);
  WHL_setSpeed(&g_DRV_context.wheelRearRight , p_speed);
  WHL_setSpeed(&g_DRV_context.wheelRearLeft  , p_speed);

  return;
}

static void DRV_moveForwardRight(uint32_t p_speed)
{
  LOG_debug("Moving forward-right @%u", p_speed);

  DRV_setDirectionsForwardRight();

  WHL_setSpeed(&g_DRV_context.wheelFrontRight,       0);
  WHL_setSpeed(&g_DRV_context.wheelFrontLeft , p_speed);
  WHL_setSpeed(&g_DRV_context.wheelRearRight , p_speed);
  WHL_setSpeed(&g_DRV_context.wheelRearLeft  ,       0);

  return;
}

static void DRV_moveForwardLeft(uint32_t p_speed)
{
  LOG_debug("Moving forward-left @%u", p_speed);

  DRV_setDirectionsForwardLeft();

  WHL_setSpeed(&g_DRV_context.wheelFrontRight, p_speed);
  WHL_setSpeed(&g_DRV_context.wheelFrontLeft ,       0);
  WHL_setSpeed(&g_DRV_context.wheelRearRight ,       0);
  WHL_setSpeed(&g_DRV_context.wheelRearLeft  , p_speed);

  return;
}

static void DRV_moveBackwardRight(uint32_t p_speed)
{
  LOG_debug("Moving backward-right @%u", p_speed);

  DRV_setDirectionsBackwardRight();

  WHL_setSpeed(&g_DRV_context.wheelFrontRight, p_speed);
  WHL_setSpeed(&g_DRV_context.wheelFrontLeft ,       0);
  WHL_setSpeed(&g_DRV_context.wheelRearRight ,       0);
  WHL_setSpeed(&g_DRV_context.wheelRearLeft  , p_speed);

  return;
}

static void DRV_moveBackwardLeft(uint32_t p_speed)
{
  LOG_debug("Moving backward-left @%u", p_speed);

  DRV_setDirectionsBackwardLeft();

  WHL_setSpeed(&g_DRV_context.wheelFrontRight,       0);
  WHL_setSpeed(&g_DRV_context.wheelFrontLeft , p_speed);
  WHL_setSpeed(&g_DRV_context.wheelRearRight , p_speed);
  WHL_setSpeed(&g_DRV_context.wheelRearLeft  ,       0);

  return;
}

static void DRV_turnLeft(uint32_t p_speed)
{
  LOG_debug("Turning left @%u", p_speed);

  DRV_setDirectionsTurnLeft();

  WHL_setSpeed(&g_DRV_context.wheelFrontRight, p_speed);
  WHL_setSpeed(&g_DRV_context.wheelFrontLeft , p_speed);
  WHL_setSpeed(&g_DRV_context.wheelRearRight , p_speed);
  WHL_setSpeed(&g_DRV_context.wheelRearLeft  , p_speed);

  return;
}

static void DRV_turnRight(uint32_t p_speed)
{
  LOG_debug("Turning right @%u", p_speed);

  DRV_setDirectionsTurnRight();

  WHL_setSpeed(&g_DRV_context.wheelFrontRight, p_speed);
  WHL_setSpeed(&g_DRV_context.wheelFrontLeft , p_speed);
  WHL_setSpeed(&g_DRV_context.wheelRearRight , p_speed);
  WHL_setSpeed(&g_DRV_context.wheelRearLeft  , p_speed);

  return;
}

static void DRV_translateLeft(uint32_t p_speed)
{
  LOG_debug("Translating left @%u", p_speed);

  DRV_setDirectionsTranslateLeft();

  WHL_setSpeed(&g_DRV_context.wheelFrontRight, p_speed);
  WHL_setSpeed(&g_DRV_context.wheelFrontLeft , p_speed);
  WHL_setSpeed(&g_DRV_context.wheelRearRight , p_speed);
  WHL_setSpeed(&g_DRV_context.wheelRearLeft  , p_speed);

  return;
}

static void DRV_translateRight(uint32_t p_speed)
{
  LOG_debug("Translating right @%u", p_speed);

  DRV_setDirectionsTranslateRight();

  WHL_setSpeed(&g_DRV_context.wheelFrontRight, p_speed);
  WHL_setSpeed(&g_DRV_context.wheelFrontLeft , p_speed);
  WHL_setSpeed(&g_DRV_context.wheelRearRight , p_speed);
  WHL_setSpeed(&g_DRV_context.wheelRearLeft  , p_speed);

  return;
}
