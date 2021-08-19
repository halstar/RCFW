#include <stdio.h>
#include <string.h>

#include "robot.h"

#include "stm32f1xx_hal.h"
#include "log.h"
#include "led.h"
#include "main.h"
#include "setup.h"
#include "utils.h"
#include "drive.h"
#include "console.h"
#include "string_fifo.h"
#include "battery_check.h"
#include "master_control.h"
#include "bluetooth_control.h"

static void RBT_displayRcfwBanner(void);
static void RBT_togglePrintOutput(void);
static void RBT_updateSwReset    (void);
static void RBT_updateLogSetup   (T_BLU_Data *p_data,      uint32_t p_timeInS    );
static void RBT_updateLedMode    (T_DRV_MODE  p_driveMode, uint32_t p_voltageInMv);

typedef struct T_RBT_Context
{
  T_SFO_Handle commandsFifo;
  T_BLU_Data   bluetoothData;
  uint32_t     padUpPressedStartTimeInS;
  uint32_t     padDownPressedStartTimeInS;
  uint32_t     padLeftPressedStartTimeInS;
  uint32_t     padRightPressedStartTimeInS;
  uint32_t     swResetPollingLastTimeInS;
  uint32_t     batteryPollingLastTimeInS;
  uint32_t     ledModeUpdateLastTimeInS;
  uint32_t     driveLogInfoLastTimeInS;
  uint32_t     velocityReportLastTimeInS;
} T_RBT_Context;

static T_RBT_Context g_RBT_context;

T_RBT_PRINT_OUTPUT g_RBT_printOutput;

void RBT_init(T_RBT_Handle       *p_handle,
              TIM_HandleTypeDef  *p_htim1 ,
              TIM_HandleTypeDef  *p_htim2 ,
              TIM_HandleTypeDef  *p_htim3 ,
              TIM_HandleTypeDef  *p_htim4 ,
              TIM_HandleTypeDef  *p_htim5 ,
              TIM_HandleTypeDef  *p_htim6 ,
              TIM_HandleTypeDef  *p_htim7 ,
              TIM_HandleTypeDef  *p_htim8 ,
              RTC_HandleTypeDef  *p_hrtc  ,
              ADC_HandleTypeDef  *p_hadc1 ,
              UART_HandleTypeDef *p_huart1,
              UART_HandleTypeDef *p_huart4)
{
  HAL_StatusTypeDef l_halReturnCode;

  LOG_info("Initializing robot module");

  p_handle->htim1  = p_htim1 ;
  p_handle->htim2  = p_htim2 ;
  p_handle->htim3  = p_htim3 ;
  p_handle->htim4  = p_htim4 ;
  p_handle->htim5  = p_htim5 ;
  p_handle->htim6  = p_htim6 ;
  p_handle->htim7  = p_htim7 ;
  p_handle->htim8  = p_htim8 ;
  p_handle->hrtc   = p_hrtc  ;
  p_handle->hadc1  = p_hadc1 ;
  p_handle->huart1 = p_huart1;
  p_handle->huart4 = p_huart4;

  /* Setup local variables */
  l_halReturnCode = HAL_OK;

  /* Setup global variables */
  SFO_init    (&g_RBT_context.commandsFifo );
  BLU_initData(&g_RBT_context.bluetoothData);

  g_RBT_context.padUpPressedStartTimeInS    = 0;
  g_RBT_context.padDownPressedStartTimeInS  = 0;
  g_RBT_context.padLeftPressedStartTimeInS  = 0;
  g_RBT_context.padRightPressedStartTimeInS = 0;
  g_RBT_context.swResetPollingLastTimeInS   = 0;
  g_RBT_context.batteryPollingLastTimeInS   = 0;
  g_RBT_context.ledModeUpdateLastTimeInS    = 0;
  g_RBT_context.driveLogInfoLastTimeInS     = 0;
  g_RBT_context.velocityReportLastTimeInS   = 0;
  g_RBT_printOutput                         = RBT_PRINT_OUTPUT_TO_CONSOLE;

  /* Setup console */
  CON_init(p_handle->huart1);

  /* Setup master connection */
  MAS_init(p_handle->huart4);

#ifdef DEBUG

  /* Temporary delay/workaround to deal with debugger connection issue */
  for (int i = 0; i < 10; i++)
  {
    UTI_delayMs(1000);
  }

#endif

  /* Setup and start using logs */
  LOG_init    (STP_DEFAULT_IS_LOG_ON);
  LOG_setLevel(STP_DEFAULT_LOG_LEVEL);
  LOG_info    ("Starting RCFW"      );

  /* Display RCFW banner */
  RBT_displayRcfwBanner();

  /* Initialize utilities */
  UTI_init(p_handle->htim7, p_handle->htim6, p_handle->hrtc);

  /* Initialize Timer 1 & green LED */
  l_halReturnCode = HAL_TIM_Base_Start_IT(p_handle->htim1);

  if (l_halReturnCode != HAL_OK)
  {
    LOG_error("HAL_TIM_Base_Start_IT(&htim1) returned an error code: %d", l_halReturnCode);
  }
  else
  {
    LOG_info("Started TIMER 1 (green LED)");
  }

  LED_setMode(LED_MODE_BLINK_FAST);

  /* Initialize Timers 2, 3, 4 & 5 */
  l_halReturnCode  = HAL_TIM_Encoder_Start_IT(p_handle->htim2, TIM_CHANNEL_ALL);
  l_halReturnCode |= HAL_TIM_Encoder_Start_IT(p_handle->htim3, TIM_CHANNEL_ALL);
  l_halReturnCode |= HAL_TIM_Encoder_Start_IT(p_handle->htim4, TIM_CHANNEL_ALL);
  l_halReturnCode |= HAL_TIM_Encoder_Start_IT(p_handle->htim5, TIM_CHANNEL_ALL);

  if (l_halReturnCode != HAL_OK)
  {
    LOG_error("HAL_TIM_Encoder_Start_IT(&htimX) returned error code(s): %d", l_halReturnCode);
  }
  else
  {
    LOG_info("Started TIMER 2, 3, 4, 5 (encoders)");
  }

  /* Initialize Timer 8 */
  l_halReturnCode = HAL_TIM_Base_Start(p_handle->htim8);

  if (l_halReturnCode != HAL_OK)
  {
    LOG_error("HAL_TIM_Base_Start(&htim8) returned an error code: %d", l_halReturnCode);
  }
  else
  {
    LOG_info("Started TIMER 8 (PWM channels)");
  }

  /* Initialize battery monitor */
  BAT_init(p_handle->hadc1);

  /* Initialize PWM channels */
  l_halReturnCode  = HAL_TIM_PWM_Start(p_handle->htim8, TIM_CHANNEL_1);
  l_halReturnCode |= HAL_TIM_PWM_Start(p_handle->htim8, TIM_CHANNEL_2);
  l_halReturnCode |= HAL_TIM_PWM_Start(p_handle->htim8, TIM_CHANNEL_3);
  l_halReturnCode |= HAL_TIM_PWM_Start(p_handle->htim8, TIM_CHANNEL_4);

  if (l_halReturnCode != HAL_OK)
  {
    LOG_error("HAL_TIM_PWM_Start(&htim8) returned error code(s): %d", l_halReturnCode);
  }
  else
  {
    LOG_info("Started PWM channels");
  }

  /* Initialize bluetooth control */
  BLU_init();

  /* Initialize driving module */
  DRV_init(p_handle->htim8, p_handle->htim4, p_handle->htim5, p_handle->htim2, p_handle->htim3);

  return;
}

void RBT_update(T_RBT_Handle *p_handle)
{
  T_DRV_MODE l_driveMode;
  uint32_t   l_currentTimeInS;
  uint32_t   l_voltageInMv;

  /* Setup local variables */
  l_driveMode      = STP_DEFAULT_DRIVE_MODE;
  l_currentTimeInS = 0;
  l_voltageInMv    = 0;

  l_driveMode = DRV_getMode();

  l_currentTimeInS = UTI_getTimeS();

  if ((STP_SW_RESET_POLLING_PERIOD_IN_S != 0) &&
      (l_currentTimeInS - g_RBT_context.swResetPollingLastTimeInS >= STP_SW_RESET_POLLING_PERIOD_IN_S))
  {
    RBT_updateSwReset();

    g_RBT_context.swResetPollingLastTimeInS = l_currentTimeInS;
  }
  else
  {
    ; /* Nothing to do */
  }

  if ((STP_BATTERY_POLLING_PERIOD_IN_S != 0) &&
      (l_currentTimeInS - g_RBT_context.batteryPollingLastTimeInS >= STP_BATTERY_POLLING_PERIOD_IN_S))
  {
    BAT_update(&l_voltageInMv);

    g_RBT_context.batteryPollingLastTimeInS = l_currentTimeInS;
  }
  else
  {
    ; /* Nothing to do */
  }

  if ((STP_LED_UPDATE_MODE_PERIOD_IN_S != 0) &&
      (l_currentTimeInS - g_RBT_context.ledModeUpdateLastTimeInS >= STP_LED_UPDATE_MODE_PERIOD_IN_S))
  {
    RBT_updateLedMode (l_driveMode, l_voltageInMv);

    g_RBT_context.ledModeUpdateLastTimeInS = l_currentTimeInS;
  }
  else
  {
    ; /* Nothing to do */
  }

  BLU_receiveData        (&g_RBT_context.bluetoothData);
  RBT_updateLogSetup     (&g_RBT_context.bluetoothData, l_currentTimeInS);
  DRV_updateFromBluetooth(&g_RBT_context.bluetoothData);

  CON_updateFifo(&g_RBT_context.commandsFifo);
  MAS_updateFifo(&g_RBT_context.commandsFifo);

  if ((STP_DRIVE_LOG_INFO_PERIOD_IN_S != 0) &&
      (l_currentTimeInS - g_RBT_context.driveLogInfoLastTimeInS >= STP_DRIVE_LOG_INFO_PERIOD_IN_S))
  {
    DRV_updateFromCommands(&g_RBT_context.commandsFifo, true);

    g_RBT_context.driveLogInfoLastTimeInS = l_currentTimeInS;
  }
  else
  {
    DRV_updateFromCommands(&g_RBT_context.commandsFifo, false);
  }

  if ((STP_VELOCITY_REPORT_PERIOD_IN_S != 0) &&
      (l_currentTimeInS - g_RBT_context.velocityReportLastTimeInS >= STP_VELOCITY_REPORT_PERIOD_IN_S))
  {
    DRV_reportVelocity();

    g_RBT_context.velocityReportLastTimeInS = l_currentTimeInS;
  }
  else
  {
    ; /* Nothing to do */
  }
  return;
}

static void RBT_displayRcfwBanner(void)
{
  /* Used ASCII art generator from https://patorjk.com with font called "Colossal" */
  LOG_info("");
  LOG_info("    8888888b.        .d8888b.       8888888888      888       888"  );
  LOG_info("    888   Y88b      d88P  Y88b      888             888   o   888"  );
  LOG_info("    888    888      888    888      888             888  d8b  888"  );
  LOG_info("    888   d88P      888             8888888         888 d888b 888"  );
  LOG_info("    8888888P\"       888             888             888d88888b888" );
  LOG_info("    888 T88b        888    888      888             88888P Y88888"  );
  LOG_info("    888  T88b       Y88b  d88P      888             8888P   Y8888"  );
  LOG_info("    888   T88b       \"Y8888P\"       888             888P     Y888");
  LOG_info("");

  return;
}

static void RBT_togglePrintOutput(void)
{
  if (g_RBT_printOutput == RBT_PRINT_OUTPUT_TO_CONSOLE)
  {
    LOG_info("Directing print to MASTER");

    g_RBT_printOutput = RBT_PRINT_OUTPUT_TO_MASTER;
  }
  else
  {
    LOG_info("Directing print to CONSOLE");

    g_RBT_printOutput = RBT_PRINT_OUTPUT_TO_CONSOLE;
  }

  return;
}

static void RBT_updateSwReset(void)
{
  GPIO_PinState l_pinState;

  l_pinState = HAL_GPIO_ReadPin(SW_RESET_GPIO_Port, SW_RESET_Pin);

  if (l_pinState == GPIO_PIN_SET)
  {
    ; /* Nothing to do */
  }
  else
  {
    LOG_info("SW reset will be triggered in 3s");
    HAL_Delay(1000);
    LOG_info("SW reset will be triggered in 2s");
    HAL_Delay(1000);
    LOG_info("SW reset will be triggered in 1s");
    HAL_Delay(1000);
    LOG_info("Resetting...");
    HAL_Delay(100);

    HAL_NVIC_SystemReset();
  }

  return;
}

static void RBT_updateLogSetup(T_BLU_Data *p_data, uint32_t p_timeInS)
{
  switch (p_data->button)
  {
    case BLU_BUTTON_PAD_UP:
      if (g_RBT_context.padUpPressedStartTimeInS == 0)
      {
        g_RBT_context.padUpPressedStartTimeInS = p_timeInS;

        LOG_increaseLevel();
      }
      else if (p_timeInS - g_RBT_context.padUpPressedStartTimeInS < STP_BUTTONS_DEBOUNCE_PERIOD_IN_S)
      {
        ; /* Nothing to do */
      }
      else
      {
        g_RBT_context.padUpPressedStartTimeInS = 0;
      }
      break;

    case BLU_BUTTON_PAD_DOWN:
      if (g_RBT_context.padDownPressedStartTimeInS == 0)
      {
        g_RBT_context.padDownPressedStartTimeInS = p_timeInS;

        LOG_decreaseLevel();
      }
      else if (p_timeInS - g_RBT_context.padDownPressedStartTimeInS < STP_BUTTONS_DEBOUNCE_PERIOD_IN_S)
      {
        ; /* Nothing to do */
      }
      else
      {
        g_RBT_context.padDownPressedStartTimeInS = 0;
      }
      break;

    case BLU_BUTTON_PAD_LEFT:
      if (g_RBT_context.padLeftPressedStartTimeInS == 0)
      {
        g_RBT_context.padLeftPressedStartTimeInS = p_timeInS;

        LOG_toggleOnOff();
      }
      else if (p_timeInS - g_RBT_context.padLeftPressedStartTimeInS < STP_BUTTONS_DEBOUNCE_PERIOD_IN_S)
      {
        ; /* Nothing to do */
      }
      else
      {
        g_RBT_context.padLeftPressedStartTimeInS = 0;
      }
      break;

    case BLU_BUTTON_PAD_RIGHT:
      if (g_RBT_context.padRightPressedStartTimeInS == 0)
      {
        g_RBT_context.padRightPressedStartTimeInS = p_timeInS;

        RBT_togglePrintOutput();
      }
      else if (p_timeInS - g_RBT_context.padRightPressedStartTimeInS < STP_BUTTONS_DEBOUNCE_PERIOD_IN_S)
      {
        ; /* Nothing to do */
      }
      else
      {
        g_RBT_context.padRightPressedStartTimeInS = 0;
      }
      break;

    default:
      ; /* Nothing to do */;
      break;
  }

  if ((p_data->button != BLU_BUTTON_PAD_UP) && (p_data->button != BLU_BUTTON_PAD_DOWN))
  {
    g_RBT_context.padUpPressedStartTimeInS   = 0;
    g_RBT_context.padDownPressedStartTimeInS = 0;
  }
  else
  {
    ; /* Nothing to do */;
  }

  return;
}

static void RBT_updateLedMode(T_DRV_MODE p_driveMode, uint32_t p_voltageInMv)
{
  /* Regarding LED mode, battery check is prioritary on user requests. */
  /* Ignore 0 value as we could get it at startup or while debugging.  */
  if ((p_voltageInMv != 0) && (p_voltageInMv < STP_MIN_BATTERY_LEVEL_IN_MV))
  {
    LOG_warning("Battery is getting low: %u mV", p_voltageInMv);

    LED_setMode(LED_MODE_FORCED_OFF);
  }
  else
  {
    switch (p_driveMode)
    {
      case DRV_MODE_MANUAL_FIXED_SPEED:
        LED_setMode(LED_MODE_BLINK_SLOW);
        break;

      case DRV_MODE_MANUAL_VARIABLE_SPEED:
        LED_setMode(LED_MODE_BLINK_MEDIUM);
        break;

      case DRV_MODE_MASTER_BOARD_CONTROL:
        LED_setMode(LED_MODE_BLINK_FAST);
        break;

      default:
        ; /* Nothing to do */
        break;
    }
  }

  return;
}
