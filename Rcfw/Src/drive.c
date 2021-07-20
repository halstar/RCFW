#include "drive.h"

#include "motor.h"
#include "main.h"
#include "log.h"

#include "utils.h"

MOTOR_Handle motor1, motor2, motor3, motor4;

void DRIVE_init(TIM_HandleTypeDef *htim)
{
  LOG_info("Initializing Drive module");

  motor1.dirPin1Port    = MOTOR_1_IN_1_GPIO_Port;
  motor1.dirPin1        = MOTOR_1_IN_1_Pin;
  motor1.dirPin2Port    = MOTOR_1_IN_2_GPIO_Port;
  motor1.dirPin2        = MOTOR_1_IN_2_Pin;
  motor1.pwmTimerHandle = htim;
  motor1.pwmChannel     = TIM_CHANNEL_1;

  motor2.dirPin1Port    = MOTOR_2_IN_1_GPIO_Port;
  motor2.dirPin1        = MOTOR_2_IN_1_Pin;
  motor2.dirPin2Port    = MOTOR_2_IN_2_GPIO_Port;
  motor2.dirPin2        = MOTOR_2_IN_2_Pin;
  motor2.pwmTimerHandle = htim;
  motor2.pwmChannel     = TIM_CHANNEL_2;

  motor3.dirPin1Port    = MOTOR_3_IN_1_GPIO_Port;
  motor3.dirPin1        = MOTOR_3_IN_1_Pin;
  motor3.dirPin2Port    = MOTOR_3_IN_2_GPIO_Port;
  motor3.dirPin2        = MOTOR_3_IN_2_Pin;
  motor3.pwmTimerHandle = htim;
  motor3.pwmChannel     = TIM_CHANNEL_3;

  motor4.dirPin1Port    = MOTOR_4_IN_1_GPIO_Port;
  motor4.dirPin1        = MOTOR_4_IN_1_Pin;
  motor4.dirPin2Port    = MOTOR_4_IN_2_GPIO_Port;
  motor4.dirPin2        = MOTOR_4_IN_2_Pin;
  motor4.pwmTimerHandle = htim;
  motor4.pwmChannel     = TIM_CHANNEL_4;

  MOTOR_init(&motor1, "MOTOR_1");
  MOTOR_init(&motor2, "MOTOR_2");
  MOTOR_init(&motor3, "MOTOR_3");
  MOTOR_init(&motor4, "MOTOR_4");

  MOTOR_setSpeed(&motor1, 20);
  MOTOR_setSpeed(&motor2, 20);
  MOTOR_setSpeed(&motor3, 20);
  MOTOR_setSpeed(&motor4, 20);

  MOTOR_start(&motor1);
  MOTOR_start(&motor2);
  MOTOR_start(&motor3);
  MOTOR_start(&motor4);

  while (1)
    UTILS_delayUs(60000);

  MOTOR_stop(&motor1);
  MOTOR_stop(&motor2);
  MOTOR_stop(&motor3);
  MOTOR_stop(&motor4);

  return;
}

void DRIVE_update(void)
{
  LOG_debug("Updating Drive module");

  return;
}
