#include <stdbool.h>

#include "drive.h"

#include "motor.h"
#include "main.h"
#include "log.h"
#include "utils.h"

typedef enum
{
  DRV_MODE_MANUAL_FIXED_SPEED = 0,
  DRV_MODE_MANUAL_VARIABLE_SPEED,
  DRV_MODE_MANUAL_CONTROLLED_SPEED,
  DRV_MODE_MASTER_BOARD_CONTROLLED_SPEED
} T_DRV_MODE;

#define DRV_JOYSTICKS_THRESHOLD   10
#define DRV_JOYSTICKS_FIXED_SPEED 25
/* Double buttons fixed speed at it concerns forward/backward + left/right movements, */
/* the latter actually using only 2 motors, while the other movements use 4 motors.   */
#define DRV_BUTTONS_FIXED_SPEED   (DRV_JOYSTICKS_FIXED_SPEED * 2)

static bool         g_DRV_isDebugOn;
static bool         g_DRV_isActive;
static T_DRV_MODE   g_DRV_mode;
static T_MTR_Handle g_DRV_motorFrontRight, g_DRV_motorFrontLeft, g_DRV_motorRearLeft, g_DRV_motorRearRight;

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

void DRV_init(TIM_HandleTypeDef *p_pwmTimerHandle)
{
  LOG_info("Initializing Drive module");

  g_DRV_motorFrontRight.dirPin1Port    = MOTOR_FRONT_RIGHT_IN_1_GPIO_Port;
  g_DRV_motorFrontRight.dirPin1        = MOTOR_FRONT_RIGHT_IN_1_Pin;
  g_DRV_motorFrontRight.dirPin2Port    = MOTOR_FRONT_RIGHT_IN_2_GPIO_Port;
  g_DRV_motorFrontRight.dirPin2        = MOTOR_FRONT_RIGHT_IN_2_Pin;
  g_DRV_motorFrontRight.pwmTimerHandle = p_pwmTimerHandle;
  g_DRV_motorFrontRight.pwmChannel     = TIM_CHANNEL_2;

  g_DRV_motorFrontLeft.dirPin1Port     = MOTOR_FRONT_LEFT_IN_1_GPIO_Port;
  g_DRV_motorFrontLeft.dirPin1         = MOTOR_FRONT_LEFT_IN_1_Pin;
  g_DRV_motorFrontLeft.dirPin2Port     = MOTOR_FRONT_LEFT_IN_2_GPIO_Port;
  g_DRV_motorFrontLeft.dirPin2         = MOTOR_FRONT_LEFT_IN_2_Pin;
  g_DRV_motorFrontLeft.pwmTimerHandle  = p_pwmTimerHandle;
  g_DRV_motorFrontLeft.pwmChannel      = TIM_CHANNEL_1;

  g_DRV_motorRearLeft.dirPin1Port      = MOTOR_REAR_LEFT_IN_1_GPIO_Port;
  g_DRV_motorRearLeft.dirPin1          = MOTOR_REAR_LEFT_IN_1_Pin;
  g_DRV_motorRearLeft.dirPin2Port      = MOTOR_REAR_LEFT_IN_2_GPIO_Port;
  g_DRV_motorRearLeft.dirPin2          = MOTOR_REAR_LEFT_IN_2_Pin;
  g_DRV_motorRearLeft.pwmTimerHandle   = p_pwmTimerHandle;
  g_DRV_motorRearLeft.pwmChannel       = TIM_CHANNEL_4;

  g_DRV_motorRearRight.dirPin1Port     = MOTOR_REAR_RIGHT_IN_1_GPIO_Port;
  g_DRV_motorRearRight.dirPin1         = MOTOR_REAR_RIGHT_IN_1_Pin;
  g_DRV_motorRearRight.dirPin2Port     = MOTOR_REAR_RIGHT_IN_2_GPIO_Port;
  g_DRV_motorRearRight.dirPin2         = MOTOR_REAR_RIGHT_IN_2_Pin;
  g_DRV_motorRearRight.pwmTimerHandle  = p_pwmTimerHandle;
  g_DRV_motorRearRight.pwmChannel      = TIM_CHANNEL_3;

  MTR_init(&g_DRV_motorFrontRight, "FRONT_RIGHT");
  MTR_init(&g_DRV_motorFrontLeft , "FRONT_LEFT ");
  MTR_init(&g_DRV_motorRearLeft  , "REAR_LEFT  ");
  MTR_init(&g_DRV_motorRearRight , "REAR_RIGHT ");

  /* Start motors (but with a 0 speed at this point) */
  MTR_start(&g_DRV_motorFrontRight);
  MTR_start(&g_DRV_motorFrontLeft );
  MTR_start(&g_DRV_motorRearRight );
  MTR_start(&g_DRV_motorRearLeft  );

  /* De-activate debug mode: motors will make the car move */
  g_DRV_isDebugOn = false;

  /* Considered that drive is inactive when the code starts */
  g_DRV_isActive = false;

  /* Start with master board control mode */
  g_DRV_mode = DRV_MODE_MASTER_BOARD_CONTROLLED_SPEED;

  return;
}

void DRV_update(T_BLU_Data *p_bluetoothData)
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
      if (g_DRV_mode != DRV_MODE_MANUAL_CONTROLLED_SPEED)
      {
        LOG_info("Drive mode now DRV_MODE_MANUAL_CONTROLLED_SPEED");
        g_DRV_mode = DRV_MODE_MANUAL_CONTROLLED_SPEED;
      }
      else
      {
        ; /* Nothing to do */
      }
      break;

    case BLU_BUTTON_GREEN_TRIANGLE:
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
      if (g_DRV_isDebugOn == false)
      {
        LOG_info("Drive debug mode turned ON  - Motors now OFF");
        g_DRV_isDebugOn = true;
      }
      else
      {
        ; /* Nothing to do */
      }
      break;

    case BLU_BUTTON_START:
      if (g_DRV_isDebugOn == true)
      {
        LOG_info("Drive debug mode turned OFF - Motors now ON");
        g_DRV_isDebugOn = false;
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
      DRV_sleep();
    }
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

  if (g_DRV_isDebugOn == true)
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

  if (g_DRV_isDebugOn == true)
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

  if (g_DRV_isDebugOn == true)
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

  if (g_DRV_isDebugOn == true)
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

  if (g_DRV_isDebugOn == true)
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

  if (g_DRV_isDebugOn == true)
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

  if (g_DRV_isDebugOn == true)
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

  if (g_DRV_isDebugOn == true)
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

  if (g_DRV_isDebugOn == true)
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

  if (g_DRV_isDebugOn == true)
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
