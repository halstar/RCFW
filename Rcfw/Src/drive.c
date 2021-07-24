#include <stdbool.h>

#include "drive.h"

#include "motor.h"
#include "main.h"
#include "log.h"
#include "utils.h"

typedef enum
{
  DRIVE_MODE_MANUAL_FIXED_SPEED = 0,
  DRIVE_MODE_MANUAL_VARIABLE_SPEED,
  DRIVE_MODE_MANUAL_CONTROLLED_SPEED,
  DRIVE_MODE_MASTER_BOARD_CONTROLLED_SPEED
} T_DRIVE_MODE;

#define DRIVE_JOYSTICKS_THRESHOLD   10
#define DRIVE_JOYSTICKS_FIXED_SPEED 25
/* Double buttons fixed speed at it concerns forward/backward + left/right movements, */
/* the latter actually using only 2 motors, while the other movements use 4 motors.   */
#define DRIVE_BUTTONS_FIXED_SPEED   (DRIVE_JOYSTICKS_FIXED_SPEED * 2)

static bool           DRIVE_isDebugOn;
static bool           DRIVE_isActive;
static T_DRIVE_MODE   DRIVE_mode;
static T_MOTOR_Handle DRIVE_motorFrontRight, DRIVE_motorFrontLeft, DRIVE_motorRearLeft, DRIVE_motorRearRight;

static void DRIVE_sleep            (void             );
static void DRIVE_moveForward      (uint32_t  p_speed);
static void DRIVE_moveBackward     (uint32_t  p_speed);
static void DRIVE_moveForwardRight (uint32_t  p_speed);
static void DRIVE_moveForwardLeft  (uint32_t  p_speed);
static void DRIVE_moveBackwardRight(uint32_t  p_speed);
static void DRIVE_moveBackwardLeft (uint32_t  p_speed);
static void DRIVE_turnLeft         (uint32_t  p_speed);
static void DRIVE_turnRight        (uint32_t  p_speed);
static void DRIVE_translateLeft    (uint32_t  p_speed);
static void DRIVE_translateRight   (uint32_t  p_speed);

void DRIVE_init(TIM_HandleTypeDef *p_pwmTimerHandle)
{
  LOG_info("Initializing Drive module");

  DRIVE_motorFrontRight.dirPin1Port    = MOTOR_FRONT_RIGHT_IN_1_GPIO_Port;
  DRIVE_motorFrontRight.dirPin1        = MOTOR_FRONT_RIGHT_IN_1_Pin;
  DRIVE_motorFrontRight.dirPin2Port    = MOTOR_FRONT_RIGHT_IN_2_GPIO_Port;
  DRIVE_motorFrontRight.dirPin2        = MOTOR_FRONT_RIGHT_IN_2_Pin;
  DRIVE_motorFrontRight.pwmTimerHandle = p_pwmTimerHandle;
  DRIVE_motorFrontRight.pwmChannel     = TIM_CHANNEL_2;

  DRIVE_motorFrontLeft.dirPin1Port    = MOTOR_FRONT_LEFT_IN_1_GPIO_Port;
  DRIVE_motorFrontLeft.dirPin1        = MOTOR_FRONT_LEFT_IN_1_Pin;
  DRIVE_motorFrontLeft.dirPin2Port    = MOTOR_FRONT_LEFT_IN_2_GPIO_Port;
  DRIVE_motorFrontLeft.dirPin2        = MOTOR_FRONT_LEFT_IN_2_Pin;
  DRIVE_motorFrontLeft.pwmTimerHandle = p_pwmTimerHandle;
  DRIVE_motorFrontLeft.pwmChannel     = TIM_CHANNEL_1;

  DRIVE_motorRearLeft.dirPin1Port    = MOTOR_REAR_LEFT_IN_1_GPIO_Port;
  DRIVE_motorRearLeft.dirPin1        = MOTOR_REAR_LEFT_IN_1_Pin;
  DRIVE_motorRearLeft.dirPin2Port    = MOTOR_REAR_LEFT_IN_2_GPIO_Port;
  DRIVE_motorRearLeft.dirPin2        = MOTOR_REAR_LEFT_IN_2_Pin;
  DRIVE_motorRearLeft.pwmTimerHandle = p_pwmTimerHandle;
  DRIVE_motorRearLeft.pwmChannel     = TIM_CHANNEL_4;

  DRIVE_motorRearRight.dirPin1Port    = MOTOR_REAR_RIGHT_IN_1_GPIO_Port;
  DRIVE_motorRearRight.dirPin1        = MOTOR_REAR_RIGHT_IN_1_Pin;
  DRIVE_motorRearRight.dirPin2Port    = MOTOR_REAR_RIGHT_IN_2_GPIO_Port;
  DRIVE_motorRearRight.dirPin2        = MOTOR_REAR_RIGHT_IN_2_Pin;
  DRIVE_motorRearRight.pwmTimerHandle = p_pwmTimerHandle;
  DRIVE_motorRearRight.pwmChannel     = TIM_CHANNEL_3;

  MOTOR_init(&DRIVE_motorFrontRight, "FRONT_RIGHT");
  MOTOR_init(&DRIVE_motorFrontLeft , "FRONT_LEFT ");
  MOTOR_init(&DRIVE_motorRearLeft  , "REAR_LEFT  ");
  MOTOR_init(&DRIVE_motorRearRight , "REAR_RIGHT ");

  /* Start motors (but with a 0 speed at this point) */
  MOTOR_start(&DRIVE_motorFrontRight);
  MOTOR_start(&DRIVE_motorFrontLeft );
  MOTOR_start(&DRIVE_motorRearRight );
  MOTOR_start(&DRIVE_motorRearLeft  );

  /* De-activate debug mode: motors will make the car move */
  DRIVE_isDebugOn = false;

  /* Considered that drive is inactive when the code starts */
  DRIVE_isActive = false;

  /* Start with master board control mode */
  DRIVE_mode = DRIVE_MODE_MASTER_BOARD_CONTROLLED_SPEED;

  return;
}

void DRIVE_update(T_BLUETOOTH_CONTROL_Data *p_bluetoothData)
{
  uint32_t l_speed;

  // LOG_debug("Updating Drive module");

  /* Check possible requested mode change */
  switch (p_bluetoothData->button)
  {
    case BLUETOOTH_CONTROL_BUTTON_PINK_SQUARE:
      if (DRIVE_mode != DRIVE_MODE_MANUAL_FIXED_SPEED)
      {
        LOG_info("Drive mode now DRIVE_MODE_MANUAL_FIXED_SPEED");
        DRIVE_mode = DRIVE_MODE_MANUAL_FIXED_SPEED;
      }
      else
      {
        ; /* Nothing to do */
      }
      break;

    case BLUETOOTH_CONTROL_BUTTON_BLUE_CROSS:
      if (DRIVE_mode != DRIVE_MODE_MANUAL_VARIABLE_SPEED)
      {
        LOG_info("Drive mode now DRIVE_MODE_MANUAL_VARIABLE_SPEED");
        DRIVE_mode = DRIVE_MODE_MANUAL_VARIABLE_SPEED;
      }
      else
      {
        ; /* Nothing to do */
      }
      break;

    case BLUETOOTH_CONTROL_BUTTON_RED_CIRCLE:
      if (DRIVE_mode != DRIVE_MODE_MANUAL_CONTROLLED_SPEED)
      {
        LOG_info("Drive mode now DRIVE_MODE_MANUAL_CONTROLLED_SPEED");
        DRIVE_mode = DRIVE_MODE_MANUAL_CONTROLLED_SPEED;
      }
      else
      {
        ; /* Nothing to do */
      }
      break;

    case BLUETOOTH_CONTROL_BUTTON_GREEN_TRIANGLE:
      if (DRIVE_mode != DRIVE_MODE_MASTER_BOARD_CONTROLLED_SPEED)
      {
        LOG_info("Drive mode now DRIVE_MODE_MASTER_BOARD_CONTROLLED_SPEED");
        DRIVE_mode = DRIVE_MODE_MASTER_BOARD_CONTROLLED_SPEED;
      }
      else
      {
        ; /* Nothing to do */
      }
      break;

    case BLUETOOTH_CONTROL_BUTTON_SELECT:
      if (DRIVE_isDebugOn == false)
      {
        LOG_info("Drive debug mode turned ON  - Motors now OFF");
        DRIVE_isDebugOn = true;
      }
      else
      {
        ; /* Nothing to do */
      }
      break;

    case BLUETOOTH_CONTROL_BUTTON_START:
      if (DRIVE_isDebugOn == true)
      {
        LOG_info("Drive debug mode turned OFF - Motors now ON");
        DRIVE_isDebugOn = false;
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
  if (DRIVE_mode == DRIVE_MODE_MASTER_BOARD_CONTROLLED_SPEED)
  {
    ; /* Nothing to do */
  }
  /* Manual mode, applying directions received by bluetooth */
  else
  {
    if (p_bluetoothData->leftY > DRIVE_JOYSTICKS_THRESHOLD)
    {
      l_speed = DRIVE_mode == DRIVE_MODE_MANUAL_FIXED_SPEED ? DRIVE_JOYSTICKS_FIXED_SPEED : p_bluetoothData->leftY;

      DRIVE_moveForward(l_speed);
    }
    else if (p_bluetoothData->rightY > DRIVE_JOYSTICKS_THRESHOLD)
    {
      l_speed = DRIVE_mode == DRIVE_MODE_MANUAL_FIXED_SPEED ? DRIVE_JOYSTICKS_FIXED_SPEED : p_bluetoothData->rightY;

      DRIVE_moveForward(l_speed);
    }
    else if (p_bluetoothData->leftY < -DRIVE_JOYSTICKS_THRESHOLD)
    {
      l_speed = DRIVE_mode == DRIVE_MODE_MANUAL_FIXED_SPEED ? DRIVE_JOYSTICKS_FIXED_SPEED : -p_bluetoothData->leftY;

      DRIVE_moveBackward(l_speed);
    }
    else if (p_bluetoothData->rightY < -DRIVE_JOYSTICKS_THRESHOLD)
    {
      l_speed = DRIVE_mode == DRIVE_MODE_MANUAL_FIXED_SPEED ? DRIVE_JOYSTICKS_FIXED_SPEED : -p_bluetoothData->rightY;

      DRIVE_moveBackward(l_speed);
    }
    else if (p_bluetoothData->leftX < -DRIVE_JOYSTICKS_THRESHOLD)
    {
      l_speed = DRIVE_mode == DRIVE_MODE_MANUAL_FIXED_SPEED ? DRIVE_JOYSTICKS_FIXED_SPEED : -p_bluetoothData->leftX;

      DRIVE_turnLeft(l_speed);
    }
    else if (p_bluetoothData->leftX > DRIVE_JOYSTICKS_THRESHOLD)
    {
      l_speed = DRIVE_mode == DRIVE_MODE_MANUAL_FIXED_SPEED ? DRIVE_JOYSTICKS_FIXED_SPEED : p_bluetoothData->leftX;

      DRIVE_turnRight(l_speed);
    }
    else if (p_bluetoothData->rightX < -DRIVE_JOYSTICKS_THRESHOLD)
    {
      l_speed = DRIVE_mode == DRIVE_MODE_MANUAL_FIXED_SPEED ? DRIVE_JOYSTICKS_FIXED_SPEED : -p_bluetoothData->rightX;

      DRIVE_translateLeft(l_speed);
    }
    else if (p_bluetoothData->rightX > DRIVE_JOYSTICKS_THRESHOLD)
    {
      l_speed = DRIVE_mode == DRIVE_MODE_MANUAL_FIXED_SPEED ? DRIVE_JOYSTICKS_FIXED_SPEED : p_bluetoothData->rightX;

      DRIVE_translateRight(l_speed);
    }
    else if (p_bluetoothData->button == BLUETOOTH_CONTROL_BUTTON_L1)
    {
      DRIVE_moveForwardLeft(DRIVE_BUTTONS_FIXED_SPEED);
    }
    else if (p_bluetoothData->button == BLUETOOTH_CONTROL_BUTTON_L2)
    {
      DRIVE_moveBackwardRight(DRIVE_BUTTONS_FIXED_SPEED);
    }
    else if (p_bluetoothData->button == BLUETOOTH_CONTROL_BUTTON_R1)
    {
      DRIVE_moveForwardRight(DRIVE_BUTTONS_FIXED_SPEED);
    }
    else if (p_bluetoothData->button == BLUETOOTH_CONTROL_BUTTON_R2)
    {
      DRIVE_moveBackwardLeft(DRIVE_BUTTONS_FIXED_SPEED);
    }
    else
    {
      DRIVE_sleep();
    }
  }

  return;
}

static void DRIVE_sleep(void)
{
  if (DRIVE_isActive == true)
  {
    LOG_info("Drive going to sleep");

    MOTOR_setSpeed(&DRIVE_motorFrontRight, 0);
    MOTOR_setSpeed(&DRIVE_motorFrontLeft , 0);
    MOTOR_setSpeed(&DRIVE_motorRearRight , 0);
    MOTOR_setSpeed(&DRIVE_motorRearLeft  , 0);

    DRIVE_isActive = false;
  }
  else
  {
    ; /* Nothing to do */
  }

  return;
}

static void DRIVE_moveForward(uint32_t p_speed)
{
  uint32_t l_speed = p_speed;

  LOG_info("Moving forward @%u", l_speed);

  DRIVE_isActive = true;

  MOTOR_setDirection(&DRIVE_motorFrontRight, MOTOR_DIRECTION_FORWARD);
  MOTOR_setDirection(&DRIVE_motorFrontLeft , MOTOR_DIRECTION_FORWARD);
  MOTOR_setDirection(&DRIVE_motorRearRight , MOTOR_DIRECTION_FORWARD);
  MOTOR_setDirection(&DRIVE_motorRearLeft  , MOTOR_DIRECTION_FORWARD);

  if (DRIVE_isDebugOn == true)
  {
    ; /* Nothing to do */
  }
  else
  {
    MOTOR_setSpeed(&DRIVE_motorFrontRight, l_speed);
    MOTOR_setSpeed(&DRIVE_motorFrontLeft , l_speed);
    MOTOR_setSpeed(&DRIVE_motorRearRight , l_speed);
    MOTOR_setSpeed(&DRIVE_motorRearLeft  , l_speed);
  }

  return;
}

static void DRIVE_moveBackward(uint32_t p_speed)
{
  uint32_t l_speed = p_speed;

  LOG_info("Moving backward @%u", l_speed);

  DRIVE_isActive = true;

  MOTOR_setDirection(&DRIVE_motorFrontRight, MOTOR_DIRECTION_BACKWARD);
  MOTOR_setDirection(&DRIVE_motorFrontLeft , MOTOR_DIRECTION_BACKWARD);
  MOTOR_setDirection(&DRIVE_motorRearRight , MOTOR_DIRECTION_BACKWARD);
  MOTOR_setDirection(&DRIVE_motorRearLeft  , MOTOR_DIRECTION_BACKWARD);

  if (DRIVE_isDebugOn == true)
  {
    ; /* Nothing to do */
  }
  else
  {
    MOTOR_setSpeed(&DRIVE_motorFrontRight, l_speed);
    MOTOR_setSpeed(&DRIVE_motorFrontLeft , l_speed);
    MOTOR_setSpeed(&DRIVE_motorRearRight , l_speed);
    MOTOR_setSpeed(&DRIVE_motorRearLeft  , l_speed);
  }

  return;
}

static void DRIVE_moveForwardRight (uint32_t p_speed)
{
  uint32_t l_speed = p_speed;

  LOG_info("Moving forward-right @%u", l_speed);

  DRIVE_isActive = true;

  MOTOR_setDirection(&DRIVE_motorFrontLeft, MOTOR_DIRECTION_FORWARD);
  MOTOR_setDirection(&DRIVE_motorRearRight, MOTOR_DIRECTION_FORWARD);

  if (DRIVE_isDebugOn == true)
  {
    ; /* Nothing to do */
  }
  else
  {
    MOTOR_setSpeed(&DRIVE_motorFrontRight,       0);
    MOTOR_setSpeed(&DRIVE_motorFrontLeft , l_speed);
    MOTOR_setSpeed(&DRIVE_motorRearRight , l_speed);
    MOTOR_setSpeed(&DRIVE_motorRearLeft  ,       0);
  }

  return;
}

static void DRIVE_moveForwardLeft  (uint32_t p_speed)
{
  uint32_t l_speed = p_speed;

  LOG_info("Moving forward-left @%u", l_speed);

  DRIVE_isActive = true;

  MOTOR_setDirection(&DRIVE_motorFrontRight, MOTOR_DIRECTION_FORWARD);
  MOTOR_setDirection(&DRIVE_motorRearLeft  , MOTOR_DIRECTION_FORWARD);

  if (DRIVE_isDebugOn == true)
  {
    ; /* Nothing to do */
  }
  else
  {
    MOTOR_setSpeed(&DRIVE_motorFrontRight, l_speed);
    MOTOR_setSpeed(&DRIVE_motorFrontLeft ,       0);
    MOTOR_setSpeed(&DRIVE_motorRearRight ,       0);
    MOTOR_setSpeed(&DRIVE_motorRearLeft  , l_speed);
  }

  return;
}

static void DRIVE_moveBackwardRight(uint32_t p_speed)
{
  uint32_t l_speed = p_speed;

  LOG_info("Moving backward-right @%u", l_speed);

  DRIVE_isActive = true;

  MOTOR_setDirection(&DRIVE_motorFrontRight, MOTOR_DIRECTION_BACKWARD);
  MOTOR_setDirection(&DRIVE_motorRearLeft  , MOTOR_DIRECTION_BACKWARD);

  if (DRIVE_isDebugOn == true)
  {
    ; /* Nothing to do */
  }
  else
  {
    MOTOR_setSpeed(&DRIVE_motorFrontRight, l_speed);
    MOTOR_setSpeed(&DRIVE_motorFrontLeft ,       0);
    MOTOR_setSpeed(&DRIVE_motorRearRight ,       0);
    MOTOR_setSpeed(&DRIVE_motorRearLeft  , l_speed);
  }

  return;
}

static void DRIVE_moveBackwardLeft (uint32_t p_speed)
{
  uint32_t l_speed = p_speed;

  LOG_info("Moving backward-left @%u", l_speed);

  DRIVE_isActive = true;

  MOTOR_setDirection(&DRIVE_motorFrontLeft, MOTOR_DIRECTION_BACKWARD);
  MOTOR_setDirection(&DRIVE_motorRearRight, MOTOR_DIRECTION_BACKWARD);

  if (DRIVE_isDebugOn == true)
  {
    ; /* Nothing to do */
  }
  else
  {
    MOTOR_setSpeed(&DRIVE_motorFrontRight,       0);
    MOTOR_setSpeed(&DRIVE_motorFrontLeft , l_speed);
    MOTOR_setSpeed(&DRIVE_motorRearRight , l_speed);
    MOTOR_setSpeed(&DRIVE_motorRearLeft  ,       0);
  }

  return;
}

static void DRIVE_turnLeft(uint32_t p_speed)
{
  uint32_t l_speed = p_speed;

  LOG_info("Turning left @%u", l_speed);

  DRIVE_isActive = true;

  MOTOR_setDirection(&DRIVE_motorFrontRight, MOTOR_DIRECTION_FORWARD );
  MOTOR_setDirection(&DRIVE_motorFrontLeft , MOTOR_DIRECTION_BACKWARD);
  MOTOR_setDirection(&DRIVE_motorRearRight , MOTOR_DIRECTION_FORWARD );
  MOTOR_setDirection(&DRIVE_motorRearLeft  , MOTOR_DIRECTION_BACKWARD);

  if (DRIVE_isDebugOn == true)
  {
    ; /* Nothing to do */
  }
  else
  {
    MOTOR_setSpeed(&DRIVE_motorFrontRight, l_speed);
    MOTOR_setSpeed(&DRIVE_motorFrontLeft , l_speed);
    MOTOR_setSpeed(&DRIVE_motorRearRight , l_speed);
    MOTOR_setSpeed(&DRIVE_motorRearLeft  , l_speed);
  }

  return;
}

static void DRIVE_turnRight(uint32_t p_speed)
{
  uint32_t l_speed = p_speed;

  LOG_info("Turning right @%u", l_speed);

  DRIVE_isActive = true;

  MOTOR_setDirection(&DRIVE_motorFrontRight, MOTOR_DIRECTION_BACKWARD);
  MOTOR_setDirection(&DRIVE_motorFrontLeft , MOTOR_DIRECTION_FORWARD );
  MOTOR_setDirection(&DRIVE_motorRearRight , MOTOR_DIRECTION_BACKWARD);
  MOTOR_setDirection(&DRIVE_motorRearLeft  , MOTOR_DIRECTION_FORWARD );

  if (DRIVE_isDebugOn == true)
  {
    ; /* Nothing to do */
  }
  else
  {
    MOTOR_setSpeed(&DRIVE_motorFrontRight, l_speed);
    MOTOR_setSpeed(&DRIVE_motorFrontLeft , l_speed);
    MOTOR_setSpeed(&DRIVE_motorRearRight , l_speed);
    MOTOR_setSpeed(&DRIVE_motorRearLeft  , l_speed);
  }

  return;
}

static void DRIVE_translateLeft(uint32_t p_speed)
{
  uint32_t l_speed = p_speed;

  LOG_info("Translating left @%u", l_speed);

  DRIVE_isActive = true;

  MOTOR_setDirection(&DRIVE_motorFrontRight, MOTOR_DIRECTION_FORWARD );
  MOTOR_setDirection(&DRIVE_motorFrontLeft , MOTOR_DIRECTION_BACKWARD);
  MOTOR_setDirection(&DRIVE_motorRearRight , MOTOR_DIRECTION_BACKWARD);
  MOTOR_setDirection(&DRIVE_motorRearLeft  , MOTOR_DIRECTION_FORWARD );

  if (DRIVE_isDebugOn == true)
  {
    ; /* Nothing to do */
  }
  else
  {
    MOTOR_setSpeed(&DRIVE_motorFrontRight, l_speed);
    MOTOR_setSpeed(&DRIVE_motorFrontLeft , l_speed);
    MOTOR_setSpeed(&DRIVE_motorRearRight , l_speed);
    MOTOR_setSpeed(&DRIVE_motorRearLeft  , l_speed);
  }

  return;
}

static void DRIVE_translateRight(uint32_t p_speed)
{
  uint32_t l_speed = p_speed;

  LOG_info("Translating right @%u", l_speed);

  DRIVE_isActive = true;

  MOTOR_setDirection(&DRIVE_motorFrontRight, MOTOR_DIRECTION_BACKWARD);
  MOTOR_setDirection(&DRIVE_motorFrontLeft , MOTOR_DIRECTION_FORWARD );
  MOTOR_setDirection(&DRIVE_motorRearRight , MOTOR_DIRECTION_FORWARD );
  MOTOR_setDirection(&DRIVE_motorRearLeft  , MOTOR_DIRECTION_BACKWARD);

  if (DRIVE_isDebugOn == true)
  {
    ; /* Nothing to do */
  }
  else
  {
    MOTOR_setSpeed(&DRIVE_motorFrontRight, l_speed);
    MOTOR_setSpeed(&DRIVE_motorFrontLeft , l_speed);
    MOTOR_setSpeed(&DRIVE_motorRearRight , l_speed);
    MOTOR_setSpeed(&DRIVE_motorRearLeft  , l_speed);
  }

  return;
}
