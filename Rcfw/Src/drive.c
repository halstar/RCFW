#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

#include "drive.h"

#include "pid.h"
#include "motor.h"
#include "encoder.h"
#include "main.h"
#include "log.h"
#include "const.h"
#include "setup.h"
#include "utils.h"
#include "circular_buffer.h"
#include "string_fifo.h"

static bool          g_DRV_areMotorsOn;
static T_DRV_MODE    g_DRV_mode;
static T_PID_Handle  g_DRV_pidFrontRight        , g_DRV_pidFrontLeft        , g_DRV_pidRearLeft        , g_DRV_pidRearRight        ;
static T_MTR_Handle  g_DRV_motorFrontRight      , g_DRV_motorFrontLeft      , g_DRV_motorRearLeft      , g_DRV_motorRearRight      ;
static T_ENC_Handle  g_DRV_encoderFrontRight    , g_DRV_encoderFrontLeft    , g_DRV_encoderRearLeft    , g_DRV_encoderRearRight    ;
static T_CBU_Context g_DRV_speedBufferFrontRight, g_DRV_speedBufferFrontLeft, g_DRV_speedBufferRearLeft, g_DRV_speedBufferRearRight;

extern RTC_HandleTypeDef hrtc;

static void DVR_getSpeedFromCommand(char *p_string, uint32_t *p_speed);

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

  /* Setup PIDs with a target speed to 0 */
  PID_init(&g_DRV_pidFrontRight,
            CST_FRONT_RIGHT_MOTOR_NAME,
            STP_DRIVE_PID_P_FACTOR,
            STP_DRIVE_PID_I_FACTOR,
            STP_DRIVE_PID_D_FACTOR,
            0,
            STP_DRIVE_MIN_SPEED,
            STP_DRIVE_MAX_SPEED,
            STP_DRIVE_PID_ANTI_WIND_UP_FACTOR);

  PID_init(&g_DRV_pidFrontLeft,
            CST_FRONT_LEFT_MOTOR_NAME,
            STP_DRIVE_PID_P_FACTOR,
            STP_DRIVE_PID_I_FACTOR,
            STP_DRIVE_PID_D_FACTOR,
            0,
            STP_DRIVE_MIN_SPEED,
            STP_DRIVE_MAX_SPEED,
            STP_DRIVE_PID_ANTI_WIND_UP_FACTOR);

  PID_init(&g_DRV_pidRearLeft,
            CST_REAR_LEFT_MOTOR_NAME,
            STP_DRIVE_PID_P_FACTOR,
            STP_DRIVE_PID_I_FACTOR,
            STP_DRIVE_PID_D_FACTOR,
            0,
            STP_DRIVE_MIN_SPEED,
            STP_DRIVE_MAX_SPEED,
            STP_DRIVE_PID_ANTI_WIND_UP_FACTOR);

  PID_init(&g_DRV_pidRearRight,
            CST_REAR_RIGHT_MOTOR_NAME,
            STP_DRIVE_PID_P_FACTOR,
            STP_DRIVE_PID_I_FACTOR,
            STP_DRIVE_PID_D_FACTOR,
            0,
            STP_DRIVE_MIN_SPEED,
            STP_DRIVE_MAX_SPEED,
            STP_DRIVE_PID_ANTI_WIND_UP_FACTOR);

  /* Setup motors (with a 0 speed & stopped direction, at this point) */
  MTR_init(&g_DRV_motorFrontRight,
            CST_FRONT_RIGHT_MOTOR_NAME,
            MOTOR_FRONT_RIGHT_OUT_1_GPIO_Port,
            MOTOR_FRONT_RIGHT_OUT_1_Pin,
            MOTOR_FRONT_RIGHT_OUT_2_GPIO_Port,
            MOTOR_FRONT_RIGHT_OUT_2_Pin,
            p_pwmTimerHandle,
            TIM_CHANNEL_4);

  MTR_init(&g_DRV_motorFrontLeft,
            CST_FRONT_LEFT_MOTOR_NAME,
            MOTOR_FRONT_LEFT_OUT_1_GPIO_Port,
            MOTOR_FRONT_LEFT_OUT_1_Pin,
            MOTOR_FRONT_LEFT_OUT_2_GPIO_Port,
            MOTOR_FRONT_LEFT_OUT_2_Pin,
            p_pwmTimerHandle,
            TIM_CHANNEL_3);

  MTR_init(&g_DRV_motorRearLeft,
            CST_REAR_LEFT_MOTOR_NAME,
            MOTOR_REAR_LEFT_OUT_1_GPIO_Port,
            MOTOR_REAR_LEFT_OUT_1_Pin,
            MOTOR_REAR_LEFT_OUT_2_GPIO_Port,
            MOTOR_REAR_LEFT_OUT_2_Pin,
            p_pwmTimerHandle,
            TIM_CHANNEL_2);

  MTR_init(&g_DRV_motorRearRight,
            CST_REAR_RIGHT_MOTOR_NAME,
            MOTOR_REAR_RIGHT_OUT_1_GPIO_Port,
            MOTOR_REAR_RIGHT_OUT_1_Pin,
            MOTOR_REAR_RIGHT_OUT_2_GPIO_Port,
            MOTOR_REAR_RIGHT_OUT_2_Pin,
            p_pwmTimerHandle,
            TIM_CHANNEL_1);

  /* Setup encoders */
  ENC_init(&g_DRV_encoderFrontRight, CST_FRONT_RIGHT_MOTOR_NAME, true , p_frontRightEncoderTimerHandle);
  ENC_init(&g_DRV_encoderFrontLeft , CST_FRONT_LEFT_MOTOR_NAME , false, p_frontLeftEncoderTimerHandle );
  ENC_init(&g_DRV_encoderRearLeft  , CST_REAR_LEFT_MOTOR_NAME  , false, p_rearLeftEncoderTimerHandle  );
  ENC_init(&g_DRV_encoderRearRight , CST_REAR_RIGHT_MOTOR_NAME , true , p_rearRightEncoderTimerHandle );

  /* Setup speed buffers */
  CBU_init(&g_DRV_speedBufferFrontRight);
  CBU_init(&g_DRV_speedBufferFrontLeft );
  CBU_init(&g_DRV_speedBufferRearLeft  );
  CBU_init(&g_DRV_speedBufferRearRight );

  /* Activate motors by default (de-activating them is used for debug)  */
  g_DRV_areMotorsOn = true;

  /* Start with master board control mode */
  g_DRV_mode = STP_DEFAULT_DRIVE_MODE;

  return;
}

void DRV_updateEncoder(TIM_HandleTypeDef *p_encoderTimerHandle)
{
  int16_t l_count;

  l_count = __HAL_TIM_GET_COUNTER(p_encoderTimerHandle);

  /* Check the handle of the triggering timer and update encoder accordingly */
  if (p_encoderTimerHandle == g_DRV_encoderRearLeft.timerHandle)
  {
    ENC_update(&g_DRV_encoderRearLeft, l_count);

    // LOG_debug("%s encoder: %d", CST_REAR_LEFT_MOTOR_NAME, ENC_getCount(&g_DRV_encoderRearLeft));
  }
  else if (p_encoderTimerHandle == g_DRV_encoderRearRight.timerHandle)
  {
    ENC_update(&g_DRV_encoderRearRight, l_count);

    // LOG_debug("%s encoder: %d", CST_REAR_RIGHT_MOTOR_NAME, ENC_getCount(&g_DRV_encoderRearRight));
  }
  else if (p_encoderTimerHandle == g_DRV_encoderFrontRight.timerHandle)
  {
    ENC_update(&g_DRV_encoderFrontRight, l_count);

    // LOG_debug("%s encoder: %d", CST_FRONT_RIGHT_MOTOR_NAME, ENC_getCount(&g_DRV_encoderFrontRight));
  }
  else if (p_encoderTimerHandle == g_DRV_encoderFrontLeft.timerHandle)
  {
    ENC_update(&g_DRV_encoderFrontLeft, l_count);

    // LOG_debug("%s encoder: %d", CST_FRONT_LEFT_MOTOR_NAME, ENC_getCount(&g_DRV_encoderFrontLeft));
  }
  else
  {
    ; /* Nothing to do */
  }

  return;
}

void DRV_updateFromBluetooth(T_BLU_Data *p_bluetoothData)
{
  uint32_t l_speed;

  /* Check possible requested mode change */
  switch (p_bluetoothData->button)
  {
    case BLU_BUTTON_PINK_SQUARE:
      if (g_DRV_mode != DRV_MODE_MANUAL_FIXED_SPEED)
      {
        LOG_info("Drive mode now DRV_MODE_MANUAL_FIXED_SPEED");
        g_DRV_mode = DRV_MODE_MANUAL_FIXED_SPEED;
      }
      else
      {
        ; /* Nothing to do */
      }
      break;

    case BLU_BUTTON_BLUE_CROSS:
      if (g_DRV_mode != DRV_MODE_MANUAL_VARIABLE_SPEED)
      {
        LOG_info("Drive mode now DRV_MODE_MANUAL_VARIABLE_SPEED");
        g_DRV_mode = DRV_MODE_MANUAL_VARIABLE_SPEED;
      }
      else
      {
        ; /* Nothing to do */
      }
      break;

    case BLU_BUTTON_RED_CIRCLE:
      if (g_DRV_mode != DRV_MODE_MASTER_BOARD_CONTROL)
      {
        LOG_info("Drive mode now DRV_MODE_MASTER_BOARD_CONTROL");
        g_DRV_mode = DRV_MODE_MASTER_BOARD_CONTROL;
      }
      else
      {
        ; /* Nothing to do */
      }
      break;

    case BLU_BUTTON_SELECT:
      if (g_DRV_areMotorsOn == true)
      {
        LOG_info("Drive turning motor OFF");
        g_DRV_areMotorsOn = false;
      }
      else
      {
        ; /* Nothing to do */
      }
      break;

    case BLU_BUTTON_START:
      if (g_DRV_areMotorsOn == false)
      {
        LOG_info("Drive turning motors ON");
        g_DRV_areMotorsOn = true;
      }
      else
      {
        ; /* Nothing to do */
      }
      break;

    case BLU_BUTTON_NONE:
    default:
      ; /* Nothing to do */
      break;
  }

  /* Master board control mode is an automated mode, so that we will */
  /* ignore any direction/button press received via bluetooth.       */
  if (g_DRV_mode == DRV_MODE_MASTER_BOARD_CONTROL)
  {
    ; /* Nothing to do */
  }
  /* Manual mode, applying directions received by bluetooth */
  else
  {
    if (p_bluetoothData->leftY > STP_JOYSTICKS_THRESHOLD)
    {
      l_speed = g_DRV_mode == DRV_MODE_MANUAL_FIXED_SPEED ? STP_JOYSTICKS_FIXED_SPEED : p_bluetoothData->leftY;

      DRV_moveForward(l_speed);
    }
    else if (p_bluetoothData->rightY > STP_JOYSTICKS_THRESHOLD)
    {
      l_speed = g_DRV_mode == DRV_MODE_MANUAL_FIXED_SPEED ? STP_JOYSTICKS_FIXED_SPEED : p_bluetoothData->rightY;

      DRV_moveForward(l_speed);
    }
    else if (p_bluetoothData->leftY < -STP_JOYSTICKS_THRESHOLD)
    {
      l_speed = g_DRV_mode == DRV_MODE_MANUAL_FIXED_SPEED ? STP_JOYSTICKS_FIXED_SPEED : -p_bluetoothData->leftY;

      DRV_moveBackward(l_speed);
    }
    else if (p_bluetoothData->rightY < -STP_JOYSTICKS_THRESHOLD)
    {
      l_speed = g_DRV_mode == DRV_MODE_MANUAL_FIXED_SPEED ? STP_JOYSTICKS_FIXED_SPEED : -p_bluetoothData->rightY;

      DRV_moveBackward(l_speed);
    }
    else if (p_bluetoothData->leftX < -STP_JOYSTICKS_THRESHOLD)
    {
      l_speed = g_DRV_mode == DRV_MODE_MANUAL_FIXED_SPEED ? STP_JOYSTICKS_FIXED_SPEED : -p_bluetoothData->leftX;

      DRV_turnLeft(l_speed);
    }
    else if (p_bluetoothData->leftX > STP_JOYSTICKS_THRESHOLD)
    {
      l_speed = g_DRV_mode == DRV_MODE_MANUAL_FIXED_SPEED ? STP_JOYSTICKS_FIXED_SPEED : p_bluetoothData->leftX;

      DRV_turnRight(l_speed);
    }
    else if (p_bluetoothData->rightX < -STP_JOYSTICKS_THRESHOLD)
    {
      l_speed = g_DRV_mode == DRV_MODE_MANUAL_FIXED_SPEED ? STP_JOYSTICKS_FIXED_SPEED : -p_bluetoothData->rightX;

      DRV_translateLeft(l_speed);
    }
    else if (p_bluetoothData->rightX > STP_JOYSTICKS_THRESHOLD)
    {
      l_speed = g_DRV_mode == DRV_MODE_MANUAL_FIXED_SPEED ? STP_JOYSTICKS_FIXED_SPEED : p_bluetoothData->rightX;

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
  }

  return;
}

void DRV_updateFromCommands(T_SFO_Context *p_commandsFifo, uint32_t p_deltaTimeInMs, bool p_logInfo)
{
  float      l_measuredSpeedFrontRight;
  float      l_measuredSpeedFrontLeft;
  float      l_measuredSpeedRearRight;
  float      l_measuredSpeedRearLeft;
  float      l_averageSpeedFrontRight;
  float      l_averageSpeedFrontLeft;
  float      l_averageSpeedRearRight;
  float      l_averageSpeedRearLeft;
  float      l_pidSpeedFrontRight;
  float      l_pidSpeedFrontLeft;
  float      l_pidSpeedRearRight;
  float      l_pidSpeedRearLeft;
  T_SFO_data l_command;
  uint32_t   l_speed;
  float      l_value;

  /* Ignore master board data only whenever a manual mode is selected */
  if (g_DRV_mode != DRV_MODE_MASTER_BOARD_CONTROL)
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

      /* Stop */
      if ((l_command[0] == 'S') && (l_command[1] == 'T'))
      {
        DRV_setDirectionsStop();

        PID_setTargetValue(&g_DRV_pidFrontRight, 0);
        PID_setTargetValue(&g_DRV_pidFrontLeft , 0);
        PID_setTargetValue(&g_DRV_pidRearRight , 0);
        PID_setTargetValue(&g_DRV_pidRearLeft  , 0);
      }
      /* Forward Straight */
      else if ((l_command[0] == 'F') && (l_command[1] == 'S'))
      {
        DVR_getSpeedFromCommand(&l_command[2], &l_speed);

        DRV_setDirectionsForward();

        PID_setTargetValue(&g_DRV_pidFrontRight, l_speed);
        PID_setTargetValue(&g_DRV_pidFrontLeft , l_speed);
        PID_setTargetValue(&g_DRV_pidRearRight , l_speed);
        PID_setTargetValue(&g_DRV_pidRearLeft  , l_speed);
      }
      /* Move Backward */
      else if ((l_command[0] == 'B') && (l_command[1] == 'S'))
      {
        DVR_getSpeedFromCommand(&l_command[2], &l_speed);

        DRV_setDirectionsBackward();

        PID_setTargetValue(&g_DRV_pidFrontRight, l_speed);
        PID_setTargetValue(&g_DRV_pidFrontLeft , l_speed);
        PID_setTargetValue(&g_DRV_pidRearRight , l_speed);
        PID_setTargetValue(&g_DRV_pidRearLeft  , l_speed);
      }
      /* TuRn (i.e. Rotate) Left */
      else if ((l_command[0] == 'R') && (l_command[1] == 'L'))
      {
        DVR_getSpeedFromCommand(&l_command[2], &l_speed);

        DRV_setDirectionsTurnLeft();

        PID_setTargetValue(&g_DRV_pidFrontRight, l_speed);
        PID_setTargetValue(&g_DRV_pidFrontLeft , l_speed);
        PID_setTargetValue(&g_DRV_pidRearRight , l_speed);
        PID_setTargetValue(&g_DRV_pidRearLeft  , l_speed);
      }
      /* TuRn (i.e. Rotate) Right */
      else if ((l_command[0] == 'R') && (l_command[1] == 'R'))
      {
        DVR_getSpeedFromCommand(&l_command[2], &l_speed);

        DRV_setDirectionsTurnRight();

        PID_setTargetValue(&g_DRV_pidFrontRight, l_speed);
        PID_setTargetValue(&g_DRV_pidFrontLeft , l_speed);
        PID_setTargetValue(&g_DRV_pidRearRight , l_speed);
        PID_setTargetValue(&g_DRV_pidRearLeft  , l_speed);
      }
      /* Translate Left */
      else if ((l_command[0] == 'T') && (l_command[1] == 'L'))
      {
        DVR_getSpeedFromCommand(&l_command[2], &l_speed);

        DRV_setDirectionsTranslateLeft();

        PID_setTargetValue(&g_DRV_pidFrontRight, l_speed);
        PID_setTargetValue(&g_DRV_pidFrontLeft , l_speed);
        PID_setTargetValue(&g_DRV_pidRearRight , l_speed);
        PID_setTargetValue(&g_DRV_pidRearLeft  , l_speed);
      }
      /* Translate Right */
      else if ((l_command[0] == 'T') && (l_command[1] == 'R'))
      {
        DVR_getSpeedFromCommand(&l_command[2], &l_speed);

        DRV_setDirectionsTranslateRight();

        PID_setTargetValue(&g_DRV_pidFrontRight, l_speed);
        PID_setTargetValue(&g_DRV_pidFrontLeft , l_speed);
        PID_setTargetValue(&g_DRV_pidRearRight , l_speed);
        PID_setTargetValue(&g_DRV_pidRearLeft  , l_speed);
      }
      /* Forward Left */
      else if ((l_command[0] == 'F') && (l_command[1] == 'L'))
      {
        DVR_getSpeedFromCommand(&l_command[2], &l_speed);

        DRV_setDirectionsForwardLeft();

        PID_setTargetValue(&g_DRV_pidFrontRight, l_speed);
        PID_setTargetValue(&g_DRV_pidFrontLeft ,       0);
        PID_setTargetValue(&g_DRV_pidRearRight ,       0);
        PID_setTargetValue(&g_DRV_pidRearLeft  , l_speed);
      }
      /* Forward Right */
      else if ((l_command[0] == 'F') && (l_command[1] == 'R'))
      {
        DVR_getSpeedFromCommand(&l_command[2], &l_speed);

        DRV_setDirectionsForwardRight();

        PID_setTargetValue(&g_DRV_pidFrontRight,       0);
        PID_setTargetValue(&g_DRV_pidFrontLeft , l_speed);
        PID_setTargetValue(&g_DRV_pidRearRight , l_speed);
        PID_setTargetValue(&g_DRV_pidRearLeft  ,       0);
      }
      /* Backward Left */
      else if ((l_command[0] == 'B') && (l_command[1] == 'L'))
      {
        DVR_getSpeedFromCommand(&l_command[2], &l_speed);

        DRV_setDirectionsBackwardLeft();

        PID_setTargetValue(&g_DRV_pidFrontRight,       0);
        PID_setTargetValue(&g_DRV_pidFrontLeft , l_speed);
        PID_setTargetValue(&g_DRV_pidRearRight , l_speed);
        PID_setTargetValue(&g_DRV_pidRearLeft  ,       0);
      }
      /* Forward Right */
      else if ((l_command[0] == 'B') && (l_command[1] == 'R'))
      {
        DVR_getSpeedFromCommand(&l_command[2], &l_speed);

        DRV_setDirectionsBackwardRight();

        PID_setTargetValue(&g_DRV_pidFrontRight, l_speed);
        PID_setTargetValue(&g_DRV_pidFrontLeft ,       0);
        PID_setTargetValue(&g_DRV_pidRearRight ,       0);
        PID_setTargetValue(&g_DRV_pidRearLeft  , l_speed);
      }
      else if ((l_command[0] == 'K') && (l_command[1] == 'P'))
      {
        l_value = atof(&l_command[2]);

        PID_setKp(&g_DRV_pidFrontRight, l_value);
        PID_setKp(&g_DRV_pidFrontLeft , l_value);
        PID_setKp(&g_DRV_pidRearRight , l_value);
        PID_setKp(&g_DRV_pidRearLeft  , l_value);
      }
      else if ((l_command[0] == 'K') && (l_command[1] == 'I'))
      {
        l_value = atof(&l_command[2]);

        PID_setKi(&g_DRV_pidFrontRight, l_value);
        PID_setKi(&g_DRV_pidFrontLeft , l_value);
        PID_setKi(&g_DRV_pidRearRight , l_value);
        PID_setKi(&g_DRV_pidRearLeft  , l_value);
      }
      else if ((l_command[0] == 'K') && (l_command[1] == 'D'))
      {
        l_value = atof(&l_command[2]);

        PID_setKd(&g_DRV_pidFrontRight, l_value);
        PID_setKd(&g_DRV_pidFrontLeft , l_value);
        PID_setKd(&g_DRV_pidRearRight , l_value);
        PID_setKd(&g_DRV_pidRearLeft  , l_value);
      }
      else
      {
        LOG_error("Drive got unsupported command: '%s'", l_command);
      }
    }

    /* Get measurements */
    l_measuredSpeedFrontRight = fabs((float)ENC_getCount(&g_DRV_encoderFrontRight) / (float)p_deltaTimeInMs * STP_DRIVE_PID_ENCODER_TO_SPEED_FACTOR);
    l_measuredSpeedFrontLeft  = fabs((float)ENC_getCount(&g_DRV_encoderFrontLeft ) / (float)p_deltaTimeInMs * STP_DRIVE_PID_ENCODER_TO_SPEED_FACTOR);
    l_measuredSpeedRearRight  = fabs((float)ENC_getCount(&g_DRV_encoderRearRight ) / (float)p_deltaTimeInMs * STP_DRIVE_PID_ENCODER_TO_SPEED_FACTOR);
    l_measuredSpeedRearLeft   = fabs((float)ENC_getCount(&g_DRV_encoderRearLeft  ) / (float)p_deltaTimeInMs * STP_DRIVE_PID_ENCODER_TO_SPEED_FACTOR);

    CBU_push(&g_DRV_speedBufferFrontRight, l_measuredSpeedFrontRight);
    CBU_push(&g_DRV_speedBufferFrontLeft , l_measuredSpeedFrontLeft );
    CBU_push(&g_DRV_speedBufferRearRight , l_measuredSpeedRearRight );
    CBU_push(&g_DRV_speedBufferRearLeft  , l_measuredSpeedRearLeft  );

    l_averageSpeedFrontRight = CBU_getAverage(&g_DRV_speedBufferFrontRight);
    l_averageSpeedFrontLeft  = CBU_getAverage(&g_DRV_speedBufferFrontLeft );
    l_averageSpeedRearRight  = CBU_getAverage(&g_DRV_speedBufferRearRight );
    l_averageSpeedRearLeft   = CBU_getAverage(&g_DRV_speedBufferRearLeft  );

    if (p_logInfo == true)
    {
      DRV_logInfo();

      LOG_info("Average speed FR / FL / RR / RL      : %2d / %2d / %2d / %2d",
               (int32_t)l_averageSpeedFrontRight,
               (int32_t)l_averageSpeedFrontLeft,
               (int32_t)l_averageSpeedRearRight,
               (int32_t)l_averageSpeedRearLeft);

      PID_logInfo(&g_DRV_pidFrontRight, true);
      PID_logInfo(&g_DRV_pidFrontLeft , true);
      PID_logInfo(&g_DRV_pidRearRight , true);
      PID_logInfo(&g_DRV_pidRearLeft  , true);
    }

    /* Update PIDs */
    l_pidSpeedFrontRight = PID_update(&g_DRV_pidFrontRight, l_averageSpeedFrontRight, p_deltaTimeInMs);
    l_pidSpeedFrontLeft  = PID_update(&g_DRV_pidFrontLeft , l_averageSpeedFrontLeft , p_deltaTimeInMs);
    l_pidSpeedRearRight  = PID_update(&g_DRV_pidRearRight , l_averageSpeedRearRight , p_deltaTimeInMs);
    l_pidSpeedRearLeft   = PID_update(&g_DRV_pidRearLeft  , l_averageSpeedRearLeft  , p_deltaTimeInMs);

    if (g_DRV_areMotorsOn == false)
    {
      ; /* Nothing to do */
    }
    else
    {
      /* Update motors */
      MTR_setSpeed(&g_DRV_motorFrontRight, l_pidSpeedFrontRight);
      MTR_setSpeed(&g_DRV_motorFrontLeft , l_pidSpeedFrontLeft );
      MTR_setSpeed(&g_DRV_motorRearRight , l_pidSpeedRearRight );
      MTR_setSpeed(&g_DRV_motorRearLeft  , l_pidSpeedRearLeft  );
    }
  }

  return;
}

T_DRV_MODE DRV_getMode(void)
{
  return g_DRV_mode;
}

void DRV_logInfo(void)
{
  T_MTR_DIRECTION l_direction;
  uint32_t        l_speed;
  uint32_t        l_count;

  if (g_DRV_mode == DRV_MODE_MANUAL_FIXED_SPEED)
  {
    LOG_info("Drive mode  : MANUAL FIXED SPEED");
  }
  else if (g_DRV_mode == DRV_MODE_MANUAL_VARIABLE_SPEED)
  {
    LOG_info("Drive mode  : MANUAL VARIABLE SPEED");
  }
  else if (g_DRV_mode == DRV_MODE_MASTER_BOARD_CONTROL)
  {
    LOG_info("Drive mode  : MASTER BOARD CONTROL");
  }
  else
  {
    LOG_error("Unsupported drive mode: %u", g_DRV_mode);
  }

  if (g_DRV_areMotorsOn == true)
  {
    LOG_info("Drive motors: ON");
  }
  else
  {
    LOG_info("Drive motors: OFF");
  }

  l_direction = MTR_getDirection(&g_DRV_motorFrontLeft  );
  l_speed     = MTR_getSpeed    (&g_DRV_motorFrontLeft  );
  l_count     = ENC_getCount    (&g_DRV_encoderFrontLeft);

  LOG_info("%s direction / speed / count: %2u / %2u / %2u", CST_FRONT_LEFT_MOTOR_NAME, l_direction, l_speed, l_count);

  l_direction = MTR_getDirection(&g_DRV_motorFrontRight  );
  l_speed     = MTR_getSpeed    (&g_DRV_motorFrontRight  );
  l_count     = ENC_getCount    (&g_DRV_encoderFrontRight);

  LOG_info("%s direction / speed / count: %2u / %2u / %2u", CST_FRONT_RIGHT_MOTOR_NAME, l_direction, l_speed, l_count);

  l_direction = MTR_getDirection(&g_DRV_motorRearRight  );
  l_speed     = MTR_getSpeed    (&g_DRV_motorRearRight  );
  l_count     = ENC_getCount    (&g_DRV_encoderRearRight);

  LOG_info("%s direction / speed / count: %2u / %2u / %2u", CST_REAR_RIGHT_MOTOR_NAME, l_direction, l_speed, l_count);

  l_direction = MTR_getDirection(&g_DRV_motorRearLeft  );
  l_speed     = MTR_getSpeed    (&g_DRV_motorRearLeft  );
  l_count     = ENC_getCount    (&g_DRV_encoderRearLeft);

  LOG_info("%s direction / speed / count: %2u / %2u / %2u", CST_REAR_LEFT_MOTOR_NAME, l_direction, l_speed, l_count);

  return;
}

static void DVR_getSpeedFromCommand(char *p_string, uint32_t *p_speed)
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

static void DRV_setDirectionsStop(void)
{
  MTR_setDirection(&g_DRV_motorFrontRight, MTR_DIRECTION_STOP);
  MTR_setDirection(&g_DRV_motorFrontLeft , MTR_DIRECTION_STOP);
  MTR_setDirection(&g_DRV_motorRearRight , MTR_DIRECTION_STOP);
  MTR_setDirection(&g_DRV_motorRearLeft  , MTR_DIRECTION_STOP);

  return;
}

static void DRV_setDirectionsForward(void)
{
  MTR_setDirection(&g_DRV_motorFrontRight, MTR_DIRECTION_FORWARD);
  MTR_setDirection(&g_DRV_motorFrontLeft , MTR_DIRECTION_FORWARD);
  MTR_setDirection(&g_DRV_motorRearRight , MTR_DIRECTION_FORWARD);
  MTR_setDirection(&g_DRV_motorRearLeft  , MTR_DIRECTION_FORWARD);

  return;
}

static void DRV_setDirectionsBackward(void)
{
  MTR_setDirection(&g_DRV_motorFrontRight, MTR_DIRECTION_BACKWARD);
  MTR_setDirection(&g_DRV_motorFrontLeft , MTR_DIRECTION_BACKWARD);
  MTR_setDirection(&g_DRV_motorRearRight , MTR_DIRECTION_BACKWARD);
  MTR_setDirection(&g_DRV_motorRearLeft  , MTR_DIRECTION_BACKWARD);

  return;
}

static void DRV_setDirectionsForwardRight(void)
{
  MTR_setDirection(&g_DRV_motorFrontLeft, MTR_DIRECTION_FORWARD);
  MTR_setDirection(&g_DRV_motorRearRight, MTR_DIRECTION_FORWARD);

  return;
}

static void DRV_setDirectionsForwardLeft(void)
{
  MTR_setDirection(&g_DRV_motorFrontRight, MTR_DIRECTION_FORWARD);
  MTR_setDirection(&g_DRV_motorRearLeft  , MTR_DIRECTION_FORWARD);

  return;
}

static void DRV_setDirectionsBackwardRight(void)
{
  MTR_setDirection(&g_DRV_motorFrontRight, MTR_DIRECTION_BACKWARD);
  MTR_setDirection(&g_DRV_motorRearLeft  , MTR_DIRECTION_BACKWARD);

  return;
}

static void DRV_setDirectionsBackwardLeft(void)
{
  MTR_setDirection(&g_DRV_motorFrontLeft, MTR_DIRECTION_BACKWARD);
  MTR_setDirection(&g_DRV_motorRearRight, MTR_DIRECTION_BACKWARD);

  return;
}

static void DRV_setDirectionsTurnLeft(void)
{
  MTR_setDirection(&g_DRV_motorFrontRight, MTR_DIRECTION_FORWARD );
  MTR_setDirection(&g_DRV_motorFrontLeft , MTR_DIRECTION_BACKWARD);
  MTR_setDirection(&g_DRV_motorRearRight , MTR_DIRECTION_FORWARD );
  MTR_setDirection(&g_DRV_motorRearLeft  , MTR_DIRECTION_BACKWARD);

  return;
}

static void DRV_setDirectionsTurnRight(void)
{
  MTR_setDirection(&g_DRV_motorFrontRight, MTR_DIRECTION_BACKWARD);
  MTR_setDirection(&g_DRV_motorFrontLeft , MTR_DIRECTION_FORWARD );
  MTR_setDirection(&g_DRV_motorRearRight , MTR_DIRECTION_BACKWARD);
  MTR_setDirection(&g_DRV_motorRearLeft  , MTR_DIRECTION_FORWARD );

  return;
}

static void DRV_setDirectionsTranslateLeft(void)
{
  MTR_setDirection(&g_DRV_motorFrontRight, MTR_DIRECTION_FORWARD );
  MTR_setDirection(&g_DRV_motorFrontLeft , MTR_DIRECTION_BACKWARD);
  MTR_setDirection(&g_DRV_motorRearRight , MTR_DIRECTION_BACKWARD);
  MTR_setDirection(&g_DRV_motorRearLeft  , MTR_DIRECTION_FORWARD );

  return;
}

static void DRV_setDirectionsTranslateRight(void)
{
  MTR_setDirection(&g_DRV_motorFrontRight, MTR_DIRECTION_BACKWARD);
  MTR_setDirection(&g_DRV_motorFrontLeft , MTR_DIRECTION_FORWARD );
  MTR_setDirection(&g_DRV_motorRearRight , MTR_DIRECTION_FORWARD );
  MTR_setDirection(&g_DRV_motorRearLeft  , MTR_DIRECTION_BACKWARD);

  return;
}

static void DRV_stop(void)
{
  DRV_setDirectionsStop();

  MTR_setSpeed(&g_DRV_motorFrontRight, 0);
  MTR_setSpeed(&g_DRV_motorFrontLeft , 0);
  MTR_setSpeed(&g_DRV_motorRearRight , 0);
  MTR_setSpeed(&g_DRV_motorRearLeft  , 0);

  return;
}

static void DRV_moveForward(uint32_t p_speed)
{
  uint32_t l_speed = p_speed;

  LOG_debug("Moving forward @%u", l_speed);

  DRV_setDirectionsForward();

  if (g_DRV_areMotorsOn == false)
  {
    ; /* Nothing to do */
  }
  else
  {
    MTR_setSpeed(&g_DRV_motorFrontRight, l_speed);
    MTR_setSpeed(&g_DRV_motorFrontLeft , l_speed);
    MTR_setSpeed(&g_DRV_motorRearRight , l_speed);
    MTR_setSpeed(&g_DRV_motorRearLeft  , l_speed);
  }

  return;
}

static void DRV_moveBackward(uint32_t p_speed)
{
  uint32_t l_speed = p_speed;

  LOG_debug("Moving backward @%u", l_speed);

  DRV_setDirectionsBackward();

  if (g_DRV_areMotorsOn == false)
  {
    ; /* Nothing to do */
  }
  else
  {
    MTR_setSpeed(&g_DRV_motorFrontRight, l_speed);
    MTR_setSpeed(&g_DRV_motorFrontLeft , l_speed);
    MTR_setSpeed(&g_DRV_motorRearRight , l_speed);
    MTR_setSpeed(&g_DRV_motorRearLeft  , l_speed);
  }

  return;
}

static void DRV_moveForwardRight(uint32_t p_speed)
{
  uint32_t l_speed = p_speed;

  LOG_debug("Moving forward-right @%u", l_speed);

  DRV_setDirectionsForwardRight();

  if (g_DRV_areMotorsOn == false)
  {
    ; /* Nothing to do */
  }
  else
  {
    MTR_setSpeed(&g_DRV_motorFrontRight,       0);
    MTR_setSpeed(&g_DRV_motorFrontLeft , l_speed);
    MTR_setSpeed(&g_DRV_motorRearRight , l_speed);
    MTR_setSpeed(&g_DRV_motorRearLeft  ,       0);
  }

  return;
}

static void DRV_moveForwardLeft(uint32_t p_speed)
{
  uint32_t l_speed = p_speed;

  LOG_debug("Moving forward-left @%u", l_speed);

  DRV_setDirectionsForwardLeft();

  if (g_DRV_areMotorsOn == false)
  {
    ; /* Nothing to do */
  }
  else
  {
    MTR_setSpeed(&g_DRV_motorFrontRight, l_speed);
    MTR_setSpeed(&g_DRV_motorFrontLeft ,       0);
    MTR_setSpeed(&g_DRV_motorRearRight ,       0);
    MTR_setSpeed(&g_DRV_motorRearLeft  , l_speed);
  }

  return;
}

static void DRV_moveBackwardRight(uint32_t p_speed)
{
  uint32_t l_speed = p_speed;

  LOG_debug("Moving backward-right @%u", l_speed);

  DRV_setDirectionsBackwardRight();

  if (g_DRV_areMotorsOn == false)
  {
    ; /* Nothing to do */
  }
  else
  {
    MTR_setSpeed(&g_DRV_motorFrontRight, l_speed);
    MTR_setSpeed(&g_DRV_motorFrontLeft ,       0);
    MTR_setSpeed(&g_DRV_motorRearRight ,       0);
    MTR_setSpeed(&g_DRV_motorRearLeft  , l_speed);
  }

  return;
}

static void DRV_moveBackwardLeft(uint32_t p_speed)
{
  uint32_t l_speed = p_speed;

  LOG_debug("Moving backward-left @%u", l_speed);

  DRV_setDirectionsBackwardLeft();

  if (g_DRV_areMotorsOn == false)
  {
    ; /* Nothing to do */
  }
  else
  {
    MTR_setSpeed(&g_DRV_motorFrontRight,       0);
    MTR_setSpeed(&g_DRV_motorFrontLeft , l_speed);
    MTR_setSpeed(&g_DRV_motorRearRight , l_speed);
    MTR_setSpeed(&g_DRV_motorRearLeft  ,       0);
  }

  return;
}

static void DRV_turnLeft(uint32_t p_speed)
{
  uint32_t l_speed = p_speed;

  LOG_debug("Turning left @%u", l_speed);

  DRV_setDirectionsTurnLeft();

  if (g_DRV_areMotorsOn == false)
  {
    ; /* Nothing to do */
  }
  else
  {
    MTR_setSpeed(&g_DRV_motorFrontRight, l_speed);
    MTR_setSpeed(&g_DRV_motorFrontLeft , l_speed);
    MTR_setSpeed(&g_DRV_motorRearRight , l_speed);
    MTR_setSpeed(&g_DRV_motorRearLeft  , l_speed);
  }

  return;
}

static void DRV_turnRight(uint32_t p_speed)
{
  uint32_t l_speed = p_speed;

  LOG_debug("Turning right @%u", l_speed);

  DRV_setDirectionsTurnRight();

  if (g_DRV_areMotorsOn == false)
  {
    ; /* Nothing to do */
  }
  else
  {
    MTR_setSpeed(&g_DRV_motorFrontRight, l_speed);
    MTR_setSpeed(&g_DRV_motorFrontLeft , l_speed);
    MTR_setSpeed(&g_DRV_motorRearRight , l_speed);
    MTR_setSpeed(&g_DRV_motorRearLeft  , l_speed);
  }

  return;
}

static void DRV_translateLeft(uint32_t p_speed)
{
  uint32_t l_speed = p_speed;

  LOG_debug("Translating left @%u", l_speed);

  DRV_setDirectionsTranslateLeft();

  if (g_DRV_areMotorsOn == false)
  {
    ; /* Nothing to do */
  }
  else
  {
    MTR_setSpeed(&g_DRV_motorFrontRight, l_speed);
    MTR_setSpeed(&g_DRV_motorFrontLeft , l_speed);
    MTR_setSpeed(&g_DRV_motorRearRight , l_speed);
    MTR_setSpeed(&g_DRV_motorRearLeft  , l_speed);
  }

  return;
}

static void DRV_translateRight(uint32_t p_speed)
{
  uint32_t l_speed = p_speed;

  LOG_debug("Translating right @%u", l_speed);

  DRV_setDirectionsTranslateRight();

  if (g_DRV_areMotorsOn == false)
  {
    ; /* Nothing to do */
  }
  else
  {
    MTR_setSpeed(&g_DRV_motorFrontRight, l_speed);
    MTR_setSpeed(&g_DRV_motorFrontLeft , l_speed);
    MTR_setSpeed(&g_DRV_motorRearRight , l_speed);
    MTR_setSpeed(&g_DRV_motorRearLeft  , l_speed);
  }

  return;
}
