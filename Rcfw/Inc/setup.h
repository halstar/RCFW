#ifndef __SETUP_H
#define __SETUP_H

#include "log.h"
#include "main.h"

#define STP_DEFAULT_IS_LOG_ON    true
#define STP_DEFAULT_LOG_LEVEL    LOG_LEVEL_DEBUG
#define STP_DEFAULT_PRINT_OUTPUT MAIN_PRINT_OUTPUT_TO_CONSOLE

#define STP_PAD_BUTTONS_DEBOUNCE_PERIOD_IN_S 2

#define STP_MASTER_MIN_SPEED   0
#define STP_MASTER_MAX_SPEED 100
#define STP_DRIVE_MIN_SPEED    0
#define STP_DRIVE_MAX_SPEED   40

#define STD_DRIVE_PID_P_FACTOR                   1.00f
#define STD_DRIVE_PID_I_FACTOR                   1.00f
#define STD_DRIVE_PID_D_FACTOR                   0.00f
#define STD_DRIVE_PID_ANTI_WIND_UP_FACTOR        0.25f
#define STD_DRIVE_PID_ENCODER_TO_SPEED_FACTOR 75000.0f

#endif /* __SETUP_H */
