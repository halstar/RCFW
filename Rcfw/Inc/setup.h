#ifndef __SETUP_H
#define __SETUP_H

#include "log.h"
#include "main.h"
#include "drive.h"

#ifdef DEBUG

  #define STP_DEFAULT_IS_LOG_ON    true
  #define STP_DEFAULT_LOG_LEVEL    LOG_LEVEL_DEBUG
  #define STP_DEFAULT_PRINT_OUTPUT MAIN_PRINT_OUTPUT_TO_CONSOLE
  #define STP_DEFAULT_DRIVE_MODE   DRV_MODE_MANUAL_FIXED_SPEED
  #define STP_DEFAULT_MOTORS_MODE  false

#else /* RELEASE */

  #define STP_DEFAULT_IS_LOG_ON    false
  #define STP_DEFAULT_LOG_LEVEL    LOG_LEVEL_ERROR
  #define STP_DEFAULT_PRINT_OUTPUT MAIN_PRINT_OUTPUT_TO_MASTER
  #define STP_DEFAULT_DRIVE_MODE   DRV_MODE_MASTER_BOARD_CONTROL
  #define STP_DEFAULT_MOTORS_MODE  true

#endif

#define STP_BUTTONS_DEBOUNCE_PERIOD_IN_S 2

#ifdef DEBUG

  #define STP_SW_RESET_POLLING_PERIOD_IN_S 1
  #define STP_BATTERY_POLLING_PERIOD_IN_S 10
  #define STP_LED_UPDATE_MODE_PERIOD_IN_S  1
  #define STP_DRIVE_LOG_INFO_PERIOD_IN_S   5
  #define STP_VELOCITY_REPORT_PERIOD_IN_S  5

#else /* RELEASE */

  #define STP_SW_RESET_POLLING_PERIOD_IN_S 0
  #define STP_BATTERY_POLLING_PERIOD_IN_S  0
  #define STP_LED_UPDATE_MODE_PERIOD_IN_S  1
  #define STP_DRIVE_LOG_INFO_PERIOD_IN_S   0
  #define STP_VELOCITY_REPORT_PERIOD_IN_S  1

#endif

#define STP_MASTER_MIN_SPEED   0
#define STP_MASTER_MAX_SPEED 100
#define STP_DRIVE_MIN_SPEED    0
#define STP_DRIVE_MAX_SPEED   40

#define STP_DRIVE_PID_P_FACTOR                   1.00f
#define STP_DRIVE_PID_I_FACTOR                   1.00f
#define STP_DRIVE_PID_D_FACTOR                   0.00f
#define STP_DRIVE_PID_ANTI_WIND_UP_FACTOR        0.25f
#define STP_DRIVE_PID_ENCODER_TO_SPEED_FACTOR 75000.0f

#define STP_JOYSTICKS_THRESHOLD   15
#define STP_JOYSTICKS_FIXED_SPEED 20
/* Double buttons fixed speed at it concerns forward/backward + left/right movements, */
/* the latter actually using only 2 motors, while the other movements use 4 motors.   */
#define STP_BUTTONS_FIXED_SPEED   (STP_JOYSTICKS_FIXED_SPEED * 2)

#define STP_MIN_BATTERY_LEVEL_IN_MV 10000

#endif /* __SETUP_H */
