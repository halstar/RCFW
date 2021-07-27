#include <stdbool.h>

#include "drive.h"

#include "pid.h"
#include "motor.h"
#include "encoder.h"
#include "main.h"
#include "log.h"
#include "utils.h"

typedef enum
{
  DRV_MODE_MANUAL_FIXED_SPEED = 0,
  DRV_MODE_MANUAL_VARIABLE_SPEED,
  DRV_MODE_MASTER_BOARD_CONTROLLED_SPEED
} T_DRV_MODE;

#define DRV_FRONT_RIGHT_MOTOR_NAME "FRONT RIGHT"
#define DRV_FRONT_LEFT_MOTOR_NAME  "FRONT LEFT "
#define DRV_REAR_LEFT_MOTOR_NAME   "REAR LEFT  "
#define DRV_REAR_RIGHT_MOTOR_NAME  "REAR RIGHT "

#define DRV_JOYSTICKS_THRESHOLD   10
#define DRV_JOYSTICKS_FIXED_SPEED 25
/* Double buttons fixed speed at it concerns forward/backward + left/right movements, */
/* the latter actually using only 2 motors, while the other movements use 4 motors.   */
#define DRV_BUTTONS_FIXED_SPEED   (DRV_JOYSTICKS_FIXED_SPEED * 2)

static bool         g_DRV_areMotorsOn;
static bool         g_DRV_isActive;
static T_DRV_MODE   g_DRV_mode;
static T_PID_Handle g_DRV_pidFrontRight    , g_DRV_pidFrontLeft    , g_DRV_pidRearLeft    , g_DRV_pidRearRight    ;
static T_MTR_Handle g_DRV_motorFrontRight  , g_DRV_motorFrontLeft  , g_DRV_motorRearLeft  , g_DRV_motorRearRight  ;
static T_ENC_Handle g_DRV_encoderFrontRight, g_DRV_encoderFrontLeft, g_DRV_encoderRearLeft, g_DRV_encoderRearRight;

static void DRV_sleep            (void            );
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

  /* Setup PIDs */
  PID_init(&g_DRV_pidFrontRight, 1, 1, 1, 0, -100, 100, 0.5);
  PID_init(&g_DRV_pidFrontLeft , 1, 1, 1, 0, -100, 100, 0.5);
  PID_init(&g_DRV_pidRearLeft  , 1, 1, 1, 0, -100, 100, 0.5);
  PID_init(&g_DRV_pidRearRight , 1, 1, 1, 0, -100, 100, 0.5);

  /* Setup motors */
  MTR_init(&g_DRV_motorFrontRight,
           DRV_FRONT_RIGHT_MOTOR_NAME,
           MOTOR_FRONT_RIGHT_IN_1_GPIO_Port,
           MOTOR_FRONT_RIGHT_IN_1_Pin,
           MOTOR_FRONT_RIGHT_IN_2_GPIO_Port,
           MOTOR_FRONT_RIGHT_IN_2_Pin,
           p_pwmTimerHandle,
           TIM_CHANNEL_2);

  MTR_init(&g_DRV_motorFrontLeft,
           DRV_FRONT_LEFT_MOTOR_NAME,
           MOTOR_FRONT_LEFT_IN_1_GPIO_Port,
           MOTOR_FRONT_LEFT_IN_1_Pin,
           MOTOR_FRONT_LEFT_IN_2_GPIO_Port,
           MOTOR_FRONT_LEFT_IN_2_Pin,
           p_pwmTimerHandle,
           TIM_CHANNEL_1);

  MTR_init(&g_DRV_motorRearLeft,
           DRV_REAR_LEFT_MOTOR_NAME,
           MOTOR_REAR_LEFT_IN_1_GPIO_Port,
           MOTOR_REAR_LEFT_IN_1_Pin,
           MOTOR_REAR_LEFT_IN_2_GPIO_Port,
           MOTOR_REAR_LEFT_IN_2_Pin,
           p_pwmTimerHandle,
           TIM_CHANNEL_4);

  MTR_init(&g_DRV_motorRearRight,
           DRV_REAR_RIGHT_MOTOR_NAME,
           MOTOR_REAR_RIGHT_IN_1_GPIO_Port,
           MOTOR_REAR_RIGHT_IN_1_Pin,
           MOTOR_REAR_RIGHT_IN_2_GPIO_Port,
           MOTOR_REAR_RIGHT_IN_2_Pin,
           p_pwmTimerHandle,
           TIM_CHANNEL_3);

  /* Setup encoders */
  ENC_init(&g_DRV_encoderFrontRight, DRV_FRONT_RIGHT_MOTOR_NAME, true , p_frontRightEncoderTimerHandle);
  ENC_init(&g_DRV_encoderFrontLeft , DRV_FRONT_LEFT_MOTOR_NAME , false, p_frontLeftEncoderTimerHandle );
  ENC_init(&g_DRV_encoderRearLeft  , DRV_REAR_LEFT_MOTOR_NAME  , false, p_rearLeftEncoderTimerHandle  );
  ENC_init(&g_DRV_encoderRearRight , DRV_REAR_RIGHT_MOTOR_NAME , true , p_rearRightEncoderTimerHandle );

  /* Start motors (but with a 0 speed at this point) */
  MTR_start(&g_DRV_motorFrontRight);
  MTR_start(&g_DRV_motorFrontLeft );
  MTR_start(&g_DRV_motorRearRight );
  MTR_start(&g_DRV_motorRearLeft  );

  /* Activate motors by default (de-activating them is used for debug  */
  g_DRV_areMotorsOn = true;

  /* Considered that drive is inactive when the code starts */
  g_DRV_isActive = false;

  /* Start with master board control mode */
  g_DRV_mode = DRV_MODE_MASTER_BOARD_CONTROLLED_SPEED;

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

//    LOG_debug("Got %s encoder interrupt: %d", DRV_REAR_LEFT_MOTOR_NAME, ENC_getCount(&g_DRV_encoderRearLeft));
  }
  else if (p_encoderTimerHandle == g_DRV_encoderRearRight.timerHandle)
  {
    ENC_update(&g_DRV_encoderRearRight, l_count);

//    LOG_debug("Got %s encoder interrupt: %d", DRV_REAR_RIGHT_MOTOR_NAME, ENC_getCount(&g_DRV_encoderRearRight));
  }
  else if (p_encoderTimerHandle == g_DRV_encoderFrontRight.timerHandle)
  {
    ENC_update(&g_DRV_encoderFrontRight, l_count);

//    LOG_debug("Got %s encoder interrupt: %d", DRV_FRONT_RIGHT_MOTOR_NAME, ENC_getCount(&g_DRV_encoderFrontRight));
  }
  else if (p_encoderTimerHandle == g_DRV_encoderFrontLeft.timerHandle)
  {
    ENC_update(&g_DRV_encoderFrontLeft, l_count);

//    LOG_debug("Got %s encoder interrupt: %d", DRV_FRONT_LEFT_MOTOR_NAME, ENC_getCount(&g_DRV_encoderFrontLeft));
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
      if (g_DRV_mode != DRV_MODE_MASTER_BOARD_CONTROLLED_SPEED)
      {
        LOG_info("Drive mode now DRV_MODE_MASTER_BOARD_CONTROLLED_SPEED");
        g_DRV_mode = DRV_MODE_MASTER_BOARD_CONTROLLED_SPEED;
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

    default:
      ; /* Nothing to do */
      break;
  }

  /* Master board control mode is an automated mode, so that we will */
  /* ignore any direction/button press received via bluetooth.       */
  if (g_DRV_mode == DRV_MODE_MASTER_BOARD_CONTROLLED_SPEED)
  {
    ; /* Nothing to do */
  }
  /* Manual mode, applying directions received by bluetooth */
  else
  {
    if (p_bluetoothData->leftY > DRV_JOYSTICKS_THRESHOLD)
    {
      l_speed = g_DRV_mode == DRV_MODE_MANUAL_FIXED_SPEED ? DRV_JOYSTICKS_FIXED_SPEED : p_bluetoothData->leftY;

      DRV_moveForward(l_speed);
    }
    else if (p_bluetoothData->rightY > DRV_JOYSTICKS_THRESHOLD)
    {
      l_speed = g_DRV_mode == DRV_MODE_MANUAL_FIXED_SPEED ? DRV_JOYSTICKS_FIXED_SPEED : p_bluetoothData->rightY;

      DRV_moveForward(l_speed);
    }
    else if (p_bluetoothData->leftY < -DRV_JOYSTICKS_THRESHOLD)
    {
      l_speed = g_DRV_mode == DRV_MODE_MANUAL_FIXED_SPEED ? DRV_JOYSTICKS_FIXED_SPEED : -p_bluetoothData->leftY;

      DRV_moveBackward(l_speed);
    }
    else if (p_bluetoothData->rightY < -DRV_JOYSTICKS_THRESHOLD)
    {
      l_speed = g_DRV_mode == DRV_MODE_MANUAL_FIXED_SPEED ? DRV_JOYSTICKS_FIXED_SPEED : -p_bluetoothData->rightY;

      DRV_moveBackward(l_speed);
    }
    else if (p_bluetoothData->leftX < -DRV_JOYSTICKS_THRESHOLD)
    {
      l_speed = g_DRV_mode == DRV_MODE_MANUAL_FIXED_SPEED ? DRV_JOYSTICKS_FIXED_SPEED : -p_bluetoothData->leftX;

      DRV_turnLeft(l_speed);
    }
    else if (p_bluetoothData->leftX > DRV_JOYSTICKS_THRESHOLD)
    {
      l_speed = g_DRV_mode == DRV_MODE_MANUAL_FIXED_SPEED ? DRV_JOYSTICKS_FIXED_SPEED : p_bluetoothData->leftX;

      DRV_turnRight(l_speed);
    }
    else if (p_bluetoothData->rightX < -DRV_JOYSTICKS_THRESHOLD)
    {
      l_speed = g_DRV_mode == DRV_MODE_MANUAL_FIXED_SPEED ? DRV_JOYSTICKS_FIXED_SPEED : -p_bluetoothData->rightX;

      DRV_translateLeft(l_speed);
    }
    else if (p_bluetoothData->rightX > DRV_JOYSTICKS_THRESHOLD)
    {
      l_speed = g_DRV_mode == DRV_MODE_MANUAL_FIXED_SPEED ? DRV_JOYSTICKS_FIXED_SPEED : p_bluetoothData->rightX;

      DRV_translateRight(l_speed);
    }
    else if (p_bluetoothData->button == BLU_BUTTON_L1)
    {
      DRV_moveForwardLeft(DRV_BUTTONS_FIXED_SPEED);
    }
    else if (p_bluetoothData->button == BLU_BUTTON_L2)
    {
      DRV_moveBackwardRight(DRV_BUTTONS_FIXED_SPEED);
    }
    else if (p_bluetoothData->button == BLU_BUTTON_R1)
    {
      DRV_moveForwardRight(DRV_BUTTONS_FIXED_SPEED);
    }
    else if (p_bluetoothData->button == BLU_BUTTON_R2)
    {
      DRV_moveBackwardLeft(DRV_BUTTONS_FIXED_SPEED);
    }
    else
    {
      /* Most of the time, we will get here */
      DRV_sleep();
    }
  }

  return;
}

void DRV_updateFromMaster(uint16_t p_deltaTime)
{
  int32_t l_pidSpeedFrontRight;
  int32_t l_pidSpeedFrontLeft;
  int32_t l_pidSpeedRearLeft;
  int32_t l_pidSpeedRearRight;

  /* Ignore master board data only whenever a manual mode is selected */
  if (g_DRV_mode != DRV_MODE_MASTER_BOARD_CONTROLLED_SPEED)
  {
    ; /* Nothing to do */
  }
  else
  {
    /* Update PIDs */
    l_pidSpeedFrontRight = PID_update(&g_DRV_pidFrontRight, 0, p_deltaTime);
    l_pidSpeedFrontLeft  = PID_update(&g_DRV_pidFrontLeft , 0, p_deltaTime);
    l_pidSpeedRearLeft   = PID_update(&g_DRV_pidRearLeft  , 0, p_deltaTime);
    l_pidSpeedRearRight  = PID_update(&g_DRV_pidRearRight , 0, p_deltaTime);
  }

  return;
}

static void DRV_sleep(void)
{
  if (g_DRV_isActive == true)
  {
    LOG_info("Drive going to sleep");

    MTR_setSpeed(&g_DRV_motorFrontRight, 0);
    MTR_setSpeed(&g_DRV_motorFrontLeft , 0);
    MTR_setSpeed(&g_DRV_motorRearRight , 0);
    MTR_setSpeed(&g_DRV_motorRearLeft  , 0);

    g_DRV_isActive = false;
  }
  else
  {
    ; /* Nothing to do */
  }

  return;
}

static void DRV_moveForward(uint32_t p_speed)
{
  uint32_t l_speed = p_speed;

  LOG_info("Moving forward @%u", l_speed);

  g_DRV_isActive = true;

  MTR_setDirection(&g_DRV_motorFrontRight, MTR_DIRECTION_FORWARD);
  MTR_setDirection(&g_DRV_motorFrontLeft , MTR_DIRECTION_FORWARD);
  MTR_setDirection(&g_DRV_motorRearRight , MTR_DIRECTION_FORWARD);
  MTR_setDirection(&g_DRV_motorRearLeft  , MTR_DIRECTION_FORWARD);

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

  LOG_info("Moving backward @%u", l_speed);

  g_DRV_isActive = true;

  MTR_setDirection(&g_DRV_motorFrontRight, MTR_DIRECTION_BACKWARD);
  MTR_setDirection(&g_DRV_motorFrontLeft , MTR_DIRECTION_BACKWARD);
  MTR_setDirection(&g_DRV_motorRearRight , MTR_DIRECTION_BACKWARD);
  MTR_setDirection(&g_DRV_motorRearLeft  , MTR_DIRECTION_BACKWARD);

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

static void DRV_moveForwardRight (uint32_t p_speed)
{
  uint32_t l_speed = p_speed;

  LOG_info("Moving forward-right @%u", l_speed);

  g_DRV_isActive = true;

  MTR_setDirection(&g_DRV_motorFrontLeft, MTR_DIRECTION_FORWARD);
  MTR_setDirection(&g_DRV_motorRearRight, MTR_DIRECTION_FORWARD);

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

static void DRV_moveForwardLeft  (uint32_t p_speed)
{
  uint32_t l_speed = p_speed;

  LOG_info("Moving forward-left @%u", l_speed);

  g_DRV_isActive = true;

  MTR_setDirection(&g_DRV_motorFrontRight, MTR_DIRECTION_FORWARD);
  MTR_setDirection(&g_DRV_motorRearLeft  , MTR_DIRECTION_FORWARD);

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

  LOG_info("Moving backward-right @%u", l_speed);

  g_DRV_isActive = true;

  MTR_setDirection(&g_DRV_motorFrontRight, MTR_DIRECTION_BACKWARD);
  MTR_setDirection(&g_DRV_motorRearLeft  , MTR_DIRECTION_BACKWARD);

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

static void DRV_moveBackwardLeft (uint32_t p_speed)
{
  uint32_t l_speed = p_speed;

  LOG_info("Moving backward-left @%u", l_speed);

  g_DRV_isActive = true;

  MTR_setDirection(&g_DRV_motorFrontLeft, MTR_DIRECTION_BACKWARD);
  MTR_setDirection(&g_DRV_motorRearRight, MTR_DIRECTION_BACKWARD);

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

  LOG_info("Turning left @%u", l_speed);

  g_DRV_isActive = true;

  MTR_setDirection(&g_DRV_motorFrontRight, MTR_DIRECTION_FORWARD );
  MTR_setDirection(&g_DRV_motorFrontLeft , MTR_DIRECTION_BACKWARD);
  MTR_setDirection(&g_DRV_motorRearRight , MTR_DIRECTION_FORWARD );
  MTR_setDirection(&g_DRV_motorRearLeft  , MTR_DIRECTION_BACKWARD);

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

  LOG_info("Turning right @%u", l_speed);

  g_DRV_isActive = true;

  MTR_setDirection(&g_DRV_motorFrontRight, MTR_DIRECTION_BACKWARD);
  MTR_setDirection(&g_DRV_motorFrontLeft , MTR_DIRECTION_FORWARD );
  MTR_setDirection(&g_DRV_motorRearRight , MTR_DIRECTION_BACKWARD);
  MTR_setDirection(&g_DRV_motorRearLeft  , MTR_DIRECTION_FORWARD );

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

  LOG_info("Translating left @%u", l_speed);

  g_DRV_isActive = true;

  MTR_setDirection(&g_DRV_motorFrontRight, MTR_DIRECTION_FORWARD );
  MTR_setDirection(&g_DRV_motorFrontLeft , MTR_DIRECTION_BACKWARD);
  MTR_setDirection(&g_DRV_motorRearRight , MTR_DIRECTION_BACKWARD);
  MTR_setDirection(&g_DRV_motorRearLeft  , MTR_DIRECTION_FORWARD );

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

  LOG_info("Translating right @%u", l_speed);

  g_DRV_isActive = true;

  MTR_setDirection(&g_DRV_motorFrontRight, MTR_DIRECTION_BACKWARD);
  MTR_setDirection(&g_DRV_motorFrontLeft , MTR_DIRECTION_FORWARD );
  MTR_setDirection(&g_DRV_motorRearRight , MTR_DIRECTION_FORWARD );
  MTR_setDirection(&g_DRV_motorRearLeft  , MTR_DIRECTION_BACKWARD);

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
