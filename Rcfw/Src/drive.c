#include <stdbool.h>

#include "drive.h"

#include "motor.h"
#include "main.h"
#include "log.h"
#include "utils.h"

typedef enum
{
  MANUAL_FIXED_SPEED = 0,
  MANUAL_VARIABLE_SPEED,
  MANUAL_CONTROLLED_SPEED,
  MASTER_BOARD_CONTROLLED_SPEED
} T_MODE;

#define DRIVE_MAXIMUM_SPEED         50
#define DRIVE_JOYSTICKS_THRESHOLD   10
#define DRIVE_JOYSTICKS_FIXED_SPEED 25
#define DRIVE_BUTTONS_FIXED_SPEED   DRIVE_JOYSTICKS_FIXED_SPEED

static bool   DRIVE_isDebugOn;
static bool   DRIVE_isStarted;
static T_MODE DRIVE_Mode;
static  MOTOR_Handle motorFrontRight, motorFrontLeft, motorRearLeft, motorRearRight;

static void DRIVE_stop             (void           );
static void DRIVE_clampSpeed       (uint32_t *p_speed);
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

void DRIVE_init(TIM_HandleTypeDef *htim)
{
  LOG_info("Initializing Drive module");

  motorFrontRight.dirPin1Port    = MOTOR_FRONT_RIGHT_IN_1_GPIO_Port;
  motorFrontRight.dirPin1        = MOTOR_FRONT_RIGHT_IN_1_Pin;
  motorFrontRight.dirPin2Port    = MOTOR_FRONT_RIGHT_IN_2_GPIO_Port;
  motorFrontRight.dirPin2        = MOTOR_FRONT_RIGHT_IN_2_Pin;
  motorFrontRight.pwmTimerHandle = htim;
  motorFrontRight.pwmChannel     = TIM_CHANNEL_2;

  motorFrontLeft.dirPin1Port    = MOTOR_FRONT_LEFT_IN_1_GPIO_Port;
  motorFrontLeft.dirPin1        = MOTOR_FRONT_LEFT_IN_1_Pin;
  motorFrontLeft.dirPin2Port    = MOTOR_FRONT_LEFT_IN_2_GPIO_Port;
  motorFrontLeft.dirPin2        = MOTOR_FRONT_LEFT_IN_2_Pin;
  motorFrontLeft.pwmTimerHandle = htim;
  motorFrontLeft.pwmChannel     = TIM_CHANNEL_1;

  motorRearLeft.dirPin1Port    = MOTOR_REAR_LEFT_IN_1_GPIO_Port;
  motorRearLeft.dirPin1        = MOTOR_REAR_LEFT_IN_1_Pin;
  motorRearLeft.dirPin2Port    = MOTOR_REAR_LEFT_IN_2_GPIO_Port;
  motorRearLeft.dirPin2        = MOTOR_REAR_LEFT_IN_2_Pin;
  motorRearLeft.pwmTimerHandle = htim;
  motorRearLeft.pwmChannel     = TIM_CHANNEL_4;

  motorRearRight.dirPin1Port    = MOTOR_REAR_RIGHT_IN_1_GPIO_Port;
  motorRearRight.dirPin1        = MOTOR_REAR_RIGHT_IN_1_Pin;
  motorRearRight.dirPin2Port    = MOTOR_REAR_RIGHT_IN_2_GPIO_Port;
  motorRearRight.dirPin2        = MOTOR_REAR_RIGHT_IN_2_Pin;
  motorRearRight.pwmTimerHandle = htim;
  motorRearRight.pwmChannel     = TIM_CHANNEL_3;

  MOTOR_init(&motorFrontRight, "FRONT_RIGHT");
  MOTOR_init(&motorFrontLeft , "FRONT_LEFT" );
  MOTOR_init(&motorRearLeft  , "REAR_LEFT"  );
  MOTOR_init(&motorRearRight , "REAR_RIGHT" );

  /* Start motors (but with a 0 speed at this point) */
  MOTOR_start(&motorFrontRight);
  MOTOR_start(&motorFrontLeft );
  MOTOR_start(&motorRearRight );
  MOTOR_start(&motorRearLeft  );

  /* De-activate debug mode: motors will make the car move */
  DRIVE_isDebugOn = false;

  /* Considered that drive is stopped we the code starts */
  DRIVE_isStarted = false;

  /* Start with master board control mode */
  DRIVE_Mode = MASTER_BOARD_CONTROLLED_SPEED;

  return;
}

void DRIVE_update(BLUETOOTH_CONTROL_DATA *data)
{
  uint32_t l_speed;

  // LOG_debug("Updating Drive module");

  /* Check possible requested mode change */
  switch (data->button)
  {
    case BUTTON_PINK_SQUARE:
      LOG_info("Drive mode now MANUAL_FIXED_SPEED");
      DRIVE_Mode = MANUAL_FIXED_SPEED;
      break;

    case BUTTON_BLUE_CROSS:
      LOG_info("Drive mode now MANUAL_VARIABLE_SPEED");
      DRIVE_Mode = MANUAL_VARIABLE_SPEED;
      break;

    case BUTTON_RED_CIRCLE:
      LOG_info("Drive mode now MANUAL_CONTROLLED_SPEED");
      DRIVE_Mode = MANUAL_CONTROLLED_SPEED;
      break;

    case BUTTON_GREEN_TRIANGLE:
      LOG_info("Drive mode now MASTER_BOARD_CONTROLLED_SPEED");
      DRIVE_Mode = MASTER_BOARD_CONTROLLED_SPEED;
      break;

    case BUTTON_SELECT:
      if (DRIVE_isDebugOn == false)
      {
        LOG_info("Drive debug mode turned ON - Motors now OFF");
        DRIVE_isDebugOn = true;
      }
      else
      {
        ; /* Nothing to do */
      }
      break;

    case BUTTON_START:
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

  /* Automated mode, ignoring directions received by bluetooth */
  if (DRIVE_Mode == MASTER_BOARD_CONTROLLED_SPEED)
  {
    ; /* Nothing to do */
  }
  /* Manual mode, applying directions received by bluetooth */
  else
  {
    if (data->leftY > DRIVE_JOYSTICKS_THRESHOLD)
    {
      l_speed = DRIVE_Mode == MANUAL_FIXED_SPEED ? DRIVE_JOYSTICKS_FIXED_SPEED : data->leftY;

      DRIVE_moveForward(l_speed);
    }
    else if (data->rightY > DRIVE_JOYSTICKS_THRESHOLD)
    {
      l_speed = DRIVE_Mode == MANUAL_FIXED_SPEED ? DRIVE_JOYSTICKS_FIXED_SPEED : data->rightY;

      DRIVE_moveForward(l_speed);
    }
    else if (data->leftY < -DRIVE_JOYSTICKS_THRESHOLD)
    {
      l_speed = DRIVE_Mode == MANUAL_FIXED_SPEED ? DRIVE_JOYSTICKS_FIXED_SPEED : -data->leftY;

      DRIVE_moveBackward(l_speed);
    }
    else if (data->rightY < -DRIVE_JOYSTICKS_THRESHOLD)
    {
      l_speed = DRIVE_Mode == MANUAL_FIXED_SPEED ? DRIVE_JOYSTICKS_FIXED_SPEED : -data->rightY;

      DRIVE_moveBackward(l_speed);
    }
    else if (data->leftX < -DRIVE_JOYSTICKS_THRESHOLD)
    {
      l_speed = DRIVE_Mode == MANUAL_FIXED_SPEED ? DRIVE_JOYSTICKS_FIXED_SPEED : -data->leftX;

      DRIVE_turnLeft(l_speed);
    }
    else if (data->leftX > DRIVE_JOYSTICKS_THRESHOLD)
    {
      l_speed = DRIVE_Mode == MANUAL_FIXED_SPEED ? DRIVE_JOYSTICKS_FIXED_SPEED : data->leftX;

      DRIVE_turnRight(l_speed);
    }
    else if (data->rightX < -DRIVE_JOYSTICKS_THRESHOLD)
    {
      l_speed = DRIVE_Mode == MANUAL_FIXED_SPEED ? DRIVE_JOYSTICKS_FIXED_SPEED : -data->rightX;

      DRIVE_translateLeft(l_speed);
    }
    else if (data->rightX > DRIVE_JOYSTICKS_THRESHOLD)
    {
      l_speed = DRIVE_Mode == MANUAL_FIXED_SPEED ? DRIVE_JOYSTICKS_FIXED_SPEED : data->rightX;

      DRIVE_translateRight(l_speed);
    }
    else if (data->button == BUTTON_L1)
    {
      DRIVE_moveForwardLeft(DRIVE_BUTTONS_FIXED_SPEED);
    }
    else if (data->button == BUTTON_L2)
    {
      DRIVE_moveBackwardRight(DRIVE_BUTTONS_FIXED_SPEED);
    }
    else if (data->button == BUTTON_R1)
    {
      DRIVE_moveForwardRight(DRIVE_BUTTONS_FIXED_SPEED);
    }
    else if (data->button == BUTTON_R2)
    {
      DRIVE_moveBackwardLeft(DRIVE_BUTTONS_FIXED_SPEED);
    }
    else
    {
      DRIVE_stop();
    }
  }

  return;
}

static void DRIVE_stop(void)
{
  if (DRIVE_isStarted == true)
  {
    LOG_info("Stopping drive");

    MOTOR_setSpeed(&motorFrontRight, 0);
    MOTOR_setSpeed(&motorFrontLeft , 0);
    MOTOR_setSpeed(&motorRearRight , 0);
    MOTOR_setSpeed(&motorRearLeft  , 0);

    DRIVE_isStarted = false;
  }
  else
  {
    ; /* Nothing to do */
  }

  return;
}

static void DRIVE_clampSpeed(uint32_t *p_speed)
{
  if (*p_speed > DRIVE_MAXIMUM_SPEED)
  {
    *p_speed = DRIVE_MAXIMUM_SPEED;
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

  DRIVE_isStarted = true;

  DRIVE_clampSpeed(&l_speed);

  MOTOR_setDirection(&motorFrontRight, MOTOR_DIRECTION_FORWARD);
  MOTOR_setDirection(&motorFrontLeft , MOTOR_DIRECTION_FORWARD);
  MOTOR_setDirection(&motorRearRight , MOTOR_DIRECTION_FORWARD);
  MOTOR_setDirection(&motorRearLeft  , MOTOR_DIRECTION_FORWARD);

  if (DRIVE_isDebugOn == true)
  {
    ; /* Nothing to do */
  }
  else
  {
    MOTOR_setSpeed(&motorFrontRight, l_speed);
    MOTOR_setSpeed(&motorFrontLeft , l_speed);
    MOTOR_setSpeed(&motorRearRight , l_speed);
    MOTOR_setSpeed(&motorRearLeft  , l_speed);
  }

  return;
}

static void DRIVE_moveBackward(uint32_t p_speed)
{
  uint32_t l_speed = p_speed;

  LOG_info("Moving backward @%u", l_speed);

  DRIVE_isStarted = true;

  DRIVE_clampSpeed(&l_speed);

  MOTOR_setDirection(&motorFrontRight, MOTOR_DIRECTION_BACKWARD);
  MOTOR_setDirection(&motorFrontLeft , MOTOR_DIRECTION_BACKWARD);
  MOTOR_setDirection(&motorRearRight , MOTOR_DIRECTION_BACKWARD);
  MOTOR_setDirection(&motorRearLeft  , MOTOR_DIRECTION_BACKWARD);

  if (DRIVE_isDebugOn == true)
  {
    ; /* Nothing to do */
  }
  else
  {
    MOTOR_setSpeed(&motorFrontRight, l_speed);
    MOTOR_setSpeed(&motorFrontLeft , l_speed);
    MOTOR_setSpeed(&motorRearRight , l_speed);
    MOTOR_setSpeed(&motorRearLeft  , l_speed);
  }

  return;
}

static void DRIVE_moveForwardRight (uint32_t p_speed)
{
  uint32_t l_speed = p_speed;

  LOG_info("Moving forward-right @%u", l_speed);

  DRIVE_isStarted = true;

  DRIVE_clampSpeed(&l_speed);

  MOTOR_setDirection(&motorFrontLeft, MOTOR_DIRECTION_FORWARD);
  MOTOR_setDirection(&motorRearRight, MOTOR_DIRECTION_FORWARD);

  if (DRIVE_isDebugOn == true)
  {
    ; /* Nothing to do */
  }
  else
  {
    MOTOR_setSpeed(&motorFrontRight,       0);
    MOTOR_setSpeed(&motorFrontLeft , l_speed);
    MOTOR_setSpeed(&motorRearRight , l_speed);
    MOTOR_setSpeed(&motorRearLeft  ,       0);
  }

  return;
}

static void DRIVE_moveForwardLeft  (uint32_t p_speed)
{
  uint32_t l_speed = p_speed;

  LOG_info("Moving forward-left @%u", l_speed);

  DRIVE_isStarted = true;

  DRIVE_clampSpeed(&l_speed);

  MOTOR_setDirection(&motorFrontRight, MOTOR_DIRECTION_FORWARD);
  MOTOR_setDirection(&motorRearLeft  , MOTOR_DIRECTION_FORWARD);

  if (DRIVE_isDebugOn == true)
  {
    ; /* Nothing to do */
  }
  else
  {
    MOTOR_setSpeed(&motorFrontRight, l_speed);
    MOTOR_setSpeed(&motorFrontLeft ,       0);
    MOTOR_setSpeed(&motorRearRight ,       0);
    MOTOR_setSpeed(&motorRearLeft  , l_speed);
  }

  return;
}

static void DRIVE_moveBackwardRight(uint32_t p_speed)
{
  uint32_t l_speed = p_speed;

  LOG_info("Moving backward-right @%u", l_speed);

  DRIVE_isStarted = true;

  DRIVE_clampSpeed(&l_speed);

  MOTOR_setDirection(&motorFrontRight, MOTOR_DIRECTION_BACKWARD);
  MOTOR_setDirection(&motorRearLeft  , MOTOR_DIRECTION_BACKWARD);

  if (DRIVE_isDebugOn == true)
  {
    ; /* Nothing to do */
  }
  else
  {
    MOTOR_setSpeed(&motorFrontRight, l_speed);
    MOTOR_setSpeed(&motorFrontLeft ,       0);
    MOTOR_setSpeed(&motorRearRight ,       0);
    MOTOR_setSpeed(&motorRearLeft  , l_speed);
  }

  return;
}

static void DRIVE_moveBackwardLeft (uint32_t p_speed)
{
  uint32_t l_speed = p_speed;

  LOG_info("Moving backward-left @%u", l_speed);

  DRIVE_isStarted = true;

  DRIVE_clampSpeed(&l_speed);

  MOTOR_setDirection(&motorFrontLeft, MOTOR_DIRECTION_BACKWARD);
  MOTOR_setDirection(&motorRearRight, MOTOR_DIRECTION_BACKWARD);

  if (DRIVE_isDebugOn == true)
  {
    ; /* Nothing to do */
  }
  else
  {
    MOTOR_setSpeed(&motorFrontRight,       0);
    MOTOR_setSpeed(&motorFrontLeft , l_speed);
    MOTOR_setSpeed(&motorRearRight , l_speed);
    MOTOR_setSpeed(&motorRearLeft  ,       0);
  }

  return;
}

static void DRIVE_turnLeft(uint32_t p_speed)
{
  uint32_t l_speed = p_speed;

  LOG_info("Turning left @%u", l_speed);

  DRIVE_isStarted = true;

  DRIVE_clampSpeed(&l_speed);

  MOTOR_setDirection(&motorFrontRight, MOTOR_DIRECTION_FORWARD );
  MOTOR_setDirection(&motorFrontLeft , MOTOR_DIRECTION_BACKWARD);
  MOTOR_setDirection(&motorRearRight , MOTOR_DIRECTION_FORWARD );
  MOTOR_setDirection(&motorRearLeft  , MOTOR_DIRECTION_BACKWARD);

  if (DRIVE_isDebugOn == true)
  {
    ; /* Nothing to do */
  }
  else
  {
    MOTOR_setSpeed(&motorFrontRight, l_speed);
    MOTOR_setSpeed(&motorFrontLeft , l_speed);
    MOTOR_setSpeed(&motorRearRight , l_speed);
    MOTOR_setSpeed(&motorRearLeft  , l_speed);
  }

  return;
}

static void DRIVE_turnRight(uint32_t p_speed)
{
  uint32_t l_speed = p_speed;

  LOG_info("Turning right @%u", l_speed);

  DRIVE_isStarted = true;

  DRIVE_clampSpeed(&l_speed);

  MOTOR_setDirection(&motorFrontRight, MOTOR_DIRECTION_BACKWARD);
  MOTOR_setDirection(&motorFrontLeft , MOTOR_DIRECTION_FORWARD );
  MOTOR_setDirection(&motorRearRight , MOTOR_DIRECTION_BACKWARD);
  MOTOR_setDirection(&motorRearLeft  , MOTOR_DIRECTION_FORWARD );

  if (DRIVE_isDebugOn == true)
  {
    ; /* Nothing to do */
  }
  else
  {
    MOTOR_setSpeed(&motorFrontRight, l_speed);
    MOTOR_setSpeed(&motorFrontLeft , l_speed);
    MOTOR_setSpeed(&motorRearRight , l_speed);
    MOTOR_setSpeed(&motorRearLeft  , l_speed);
  }

  return;
}

static void DRIVE_translateLeft(uint32_t p_speed)
{
  uint32_t l_speed = p_speed;

  LOG_info("Translating left @%u", l_speed);

  DRIVE_isStarted = true;

  DRIVE_clampSpeed(&l_speed);

  MOTOR_setDirection(&motorFrontRight, MOTOR_DIRECTION_FORWARD );
  MOTOR_setDirection(&motorFrontLeft , MOTOR_DIRECTION_BACKWARD);
  MOTOR_setDirection(&motorRearRight , MOTOR_DIRECTION_BACKWARD);
  MOTOR_setDirection(&motorRearLeft  , MOTOR_DIRECTION_FORWARD );

  if (DRIVE_isDebugOn == true)
  {
    ; /* Nothing to do */
  }
  else
  {
    MOTOR_setSpeed(&motorFrontRight, l_speed);
    MOTOR_setSpeed(&motorFrontLeft , l_speed);
    MOTOR_setSpeed(&motorRearRight , l_speed);
    MOTOR_setSpeed(&motorRearLeft  , l_speed);
  }

  return;
}

static void DRIVE_translateRight(uint32_t p_speed)
{
  uint32_t l_speed = p_speed;

  LOG_info("Translating right @%u", l_speed);

  DRIVE_isStarted = true;

  DRIVE_clampSpeed(&l_speed);

  MOTOR_setDirection(&motorFrontRight, MOTOR_DIRECTION_BACKWARD);
  MOTOR_setDirection(&motorFrontLeft , MOTOR_DIRECTION_FORWARD );
  MOTOR_setDirection(&motorRearRight , MOTOR_DIRECTION_FORWARD );
  MOTOR_setDirection(&motorRearLeft  , MOTOR_DIRECTION_BACKWARD);

  if (DRIVE_isDebugOn == true)
  {
    ; /* Nothing to do */
  }
  else
  {
    MOTOR_setSpeed(&motorFrontRight, l_speed);
    MOTOR_setSpeed(&motorFrontLeft , l_speed);
    MOTOR_setSpeed(&motorRearRight , l_speed);
    MOTOR_setSpeed(&motorRearLeft  , l_speed);
  }

  return;
}
