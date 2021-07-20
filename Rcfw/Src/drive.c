#include "drive.h"

#include "motor.h"
#include "main.h"
#include "log.h"

#include "utils.h"

MOTOR_Handle motorFrontRight, motorFrontLeft, motorRearLeft, motorRearRight;

static void DRIVE_moveForward   (uint32_t speed);
static void DRIVE_moveBackward  (uint32_t speed);
static void DRIVE_turnLeft      (uint32_t speed);
static void DRIVE_turnRight     (uint32_t speed);
static void DRIVE_translateLeft (uint32_t speed);
static void DRIVE_translateRight(uint32_t speed);

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

  // DRIVE_turnLeft(20);

//  while (1)
//    UTILS_delayUs(60000);
//
  return;
}

void DRIVE_update(BLUETOOTH_CONTROL_DATA *data)
{
  LOG_debug("Updating Drive module");

  return;
}

static void DRIVE_moveForward(uint32_t speed)
{
  LOG_info("Driving forward @%u", speed);

  MOTOR_setDirection(&motorFrontRight, MOTOR_DIRECTION_FORWARD);
  MOTOR_setDirection(&motorFrontLeft , MOTOR_DIRECTION_FORWARD);
  MOTOR_setDirection(&motorRearRight , MOTOR_DIRECTION_FORWARD);
  MOTOR_setDirection(&motorRearLeft  , MOTOR_DIRECTION_FORWARD);

  MOTOR_setSpeed(&motorFrontRight, speed);
  MOTOR_setSpeed(&motorFrontLeft , speed);
  MOTOR_setSpeed(&motorRearRight , speed);
  MOTOR_setSpeed(&motorRearLeft  , speed);

  return;
}

static void DRIVE_moveBackward(uint32_t speed)
{
  LOG_info("Driving backward @%u", speed);

  MOTOR_setDirection(&motorFrontRight, MOTOR_DIRECTION_BACKWARD);
  MOTOR_setDirection(&motorFrontLeft , MOTOR_DIRECTION_BACKWARD);
  MOTOR_setDirection(&motorRearRight , MOTOR_DIRECTION_BACKWARD);
  MOTOR_setDirection(&motorRearLeft  , MOTOR_DIRECTION_BACKWARD);

  MOTOR_setSpeed(&motorFrontRight, speed);
  MOTOR_setSpeed(&motorFrontLeft , speed);
  MOTOR_setSpeed(&motorRearRight , speed);
  MOTOR_setSpeed(&motorRearLeft  , speed);

  return;
}

static void DRIVE_turnLeft(uint32_t speed)
{
  MOTOR_setDirection(&motorFrontRight, MOTOR_DIRECTION_FORWARD );
  MOTOR_setDirection(&motorFrontLeft , MOTOR_DIRECTION_BACKWARD);
  MOTOR_setDirection(&motorRearRight , MOTOR_DIRECTION_FORWARD );
  MOTOR_setDirection(&motorRearLeft  , MOTOR_DIRECTION_BACKWARD);

  MOTOR_setSpeed(&motorFrontRight, speed);
  MOTOR_setSpeed(&motorFrontLeft , speed);
  MOTOR_setSpeed(&motorRearRight , speed);
  MOTOR_setSpeed(&motorRearLeft  , speed);

  return;
}

static void DRIVE_turnRight(uint32_t speed)
{
  MOTOR_setDirection(&motorFrontRight, MOTOR_DIRECTION_BACKWARD);
  MOTOR_setDirection(&motorFrontLeft , MOTOR_DIRECTION_FORWARD );
  MOTOR_setDirection(&motorRearRight , MOTOR_DIRECTION_BACKWARD);
  MOTOR_setDirection(&motorRearLeft  , MOTOR_DIRECTION_FORWARD );

  MOTOR_setSpeed(&motorFrontRight, speed);
  MOTOR_setSpeed(&motorFrontLeft , speed);
  MOTOR_setSpeed(&motorRearRight , speed);
  MOTOR_setSpeed(&motorRearLeft  , speed);

  return;
}

static void DRIVE_translateLeft(uint32_t speed)
{
  LOG_info("Translating left @%u", speed);

  MOTOR_setDirection(&motorFrontRight, MOTOR_DIRECTION_FORWARD );
  MOTOR_setDirection(&motorFrontLeft , MOTOR_DIRECTION_BACKWARD);
  MOTOR_setDirection(&motorRearRight , MOTOR_DIRECTION_BACKWARD);
  MOTOR_setDirection(&motorRearLeft  , MOTOR_DIRECTION_FORWARD );

  MOTOR_setSpeed(&motorFrontRight, speed);
  MOTOR_setSpeed(&motorFrontLeft , speed);
  MOTOR_setSpeed(&motorRearRight , speed);
  MOTOR_setSpeed(&motorRearLeft  , speed);

  return;
}

static void DRIVE_translateRight(uint32_t speed)
{
  LOG_info("Translating right @%u", speed);

  MOTOR_setDirection(&motorFrontRight, MOTOR_DIRECTION_BACKWARD);
  MOTOR_setDirection(&motorFrontLeft , MOTOR_DIRECTION_FORWARD );
  MOTOR_setDirection(&motorRearRight , MOTOR_DIRECTION_FORWARD );
  MOTOR_setDirection(&motorRearLeft  , MOTOR_DIRECTION_BACKWARD);

  MOTOR_setSpeed(&motorFrontRight, speed);
  MOTOR_setSpeed(&motorFrontLeft , speed);
  MOTOR_setSpeed(&motorRearRight , speed);
  MOTOR_setSpeed(&motorRearLeft  , speed);

  return;
}
