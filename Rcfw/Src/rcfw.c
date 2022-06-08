#include <stdio.h>

#include "rcfw.h"

#include "stm32f1xx_hal.h"
#include "log.h"
#include "led.h"
#include "main.h"
#include "setup.h"
#include "utils.h"
#include "chrono.h"
#include "drive.h"
#include "console.h"
#include "string_fifo.h"
#include "battery_check.h"
#include "master_control.h"
#include "bluetooth_control.h"

static void RCF_displayRcfwBanner(void);
static void RCF_togglePrintOutput(void);
static void RCF_updateSwReset    (void);
static void RCF_updateLogSetup   (T_BLU_Data *p_data,      uint32_t p_timeInS    );
static void RCF_updateLedMode    (T_DRV_MODE  p_driveMode, uint32_t p_voltageInMv);

typedef struct T_RCF_Context
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
  uint32_t     velocityReportLastTimeInMs;
} T_RCF_Context;

static T_RCF_Context g_RCF_context;

T_RCF_PRINT_OUTPUT g_RCF_printOutput;

/* RTC  clock   is used  as a calendar base (e.g. for logs)                  */
/* TIM1 timer   is used  to update green LED              - PC13             */
/* TIM2 to TIM5 are used as input for motor's encoders                       */
/* TIM6 timer   is used  to produce a micro-second base for time measurement */
/* TIM7 timer   is used  to produce a micro-second base for delays           */
/* TIM8 timer   is used  to produce motors' PWM    base                      */
/* ADC1 ADC     is used  to monitor battery level         - PA5              */
/* USART1 UART  is used  for USB/serial console           - PA9  / PA10      */
/* UART4  UART  is used  to get control from master board - PC10 / PC11      */
/* PC0          is used  to trigger software reset                           */

void RCF_init(T_RCF_Handle       *p_handle,
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

  LOG_info("Initializing RCFW module");

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
  SFO_init    (&g_RCF_context.commandsFifo );
  BLU_initData(&g_RCF_context.bluetoothData);

  g_RCF_context.padUpPressedStartTimeInS    = 0;
  g_RCF_context.padDownPressedStartTimeInS  = 0;
  g_RCF_context.padLeftPressedStartTimeInS  = 0;
  g_RCF_context.padRightPressedStartTimeInS = 0;
  g_RCF_context.swResetPollingLastTimeInS   = 0;
  g_RCF_context.batteryPollingLastTimeInS   = 0;
  g_RCF_context.ledModeUpdateLastTimeInS    = 0;
  g_RCF_context.driveLogInfoLastTimeInS     = 0;
  g_RCF_context.velocityReportLastTimeInMs  = 0;
  g_RCF_printOutput                         = RCF_PRINT_OUTPUT_TO_CONSOLE;

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
  RCF_displayRcfwBanner();

  /* Initialize utilities & chronometer (used by wheels' encoders) */
  UTI_init (p_handle->htim7 , p_handle->hrtc );
  CHR_init (p_handle->htim6);
  CHR_start();

  LOG_info("Started TIMER 6 & 7 (utilities & chronometer)");

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

void RCF_update(T_RCF_Handle *p_handle)
{
  T_DRV_MODE l_driveMode;
  uint32_t   l_currentTimeInS;
  uint32_t   l_currentTimeInMs;
  uint32_t   l_voltageInMv;

  /* Setup local variables */
  l_driveMode       = STP_DEFAULT_DRIVE_MODE;
  l_currentTimeInS  = 0;
  l_currentTimeInMs = 0;
  l_voltageInMv     = 0;

  l_driveMode       = DRV_getMode  ();
  l_currentTimeInS  = UTI_getTimeS ();
  l_currentTimeInMs = UTI_getTimeMs();

  if ((STP_SW_RESET_POLLING_PERIOD_IN_S != 0) &&
      (l_currentTimeInS - g_RCF_context.swResetPollingLastTimeInS >= STP_SW_RESET_POLLING_PERIOD_IN_S))
  {
    RCF_updateSwReset();

    g_RCF_context.swResetPollingLastTimeInS = l_currentTimeInS;
  }
  else
  {
    ; /* Nothing to do */
  }

  if ((STP_BATTERY_POLLING_PERIOD_IN_S != 0) &&
      (l_currentTimeInS - g_RCF_context.batteryPollingLastTimeInS >= STP_BATTERY_POLLING_PERIOD_IN_S))
  {
    BAT_update(&l_voltageInMv);

    g_RCF_context.batteryPollingLastTimeInS = l_currentTimeInS;
  }
  else
  {
    ; /* Nothing to do */
  }

  if ((STP_LED_UPDATE_MODE_PERIOD_IN_S != 0) &&
      (l_currentTimeInS - g_RCF_context.ledModeUpdateLastTimeInS >= STP_LED_UPDATE_MODE_PERIOD_IN_S))
  {
    RCF_updateLedMode (l_driveMode, l_voltageInMv);

    g_RCF_context.ledModeUpdateLastTimeInS = l_currentTimeInS;
  }
  else
  {
    ; /* Nothing to do */
  }

  CON_updateFifo(&g_RCF_context.commandsFifo);
  MAS_updateFifo(&g_RCF_context.commandsFifo);

  BLU_receiveData   (&g_RCF_context.bluetoothData);
  RCF_updateLogSetup(&g_RCF_context.bluetoothData, l_currentTimeInS);

  DRV_updateAverageSpeeds();

  /* Restart chronometer (used to compute average speeds) */
  CHR_reset();

  if ((STP_DRIVE_LOG_INFO_PERIOD_IN_S != 0) &&
      (l_currentTimeInS - g_RCF_context.driveLogInfoLastTimeInS >= STP_DRIVE_LOG_INFO_PERIOD_IN_S))
  {
    DRV_updateFromBluetooth(&g_RCF_context.bluetoothData, true);
    DRV_updateFromCommands (&g_RCF_context.commandsFifo , true);

    g_RCF_context.driveLogInfoLastTimeInS = l_currentTimeInS;
  }
  else
  {
    DRV_updateFromBluetooth(&g_RCF_context.bluetoothData, false);
    DRV_updateFromCommands(&g_RCF_context.commandsFifo  , false);
  }

  if ((STP_VELOCITY_REPORT_PERIOD_IN_MS != 0) &&
      (l_currentTimeInMs - g_RCF_context.velocityReportLastTimeInMs >= STP_VELOCITY_REPORT_PERIOD_IN_MS))
  {
    DRV_reportVelocity();

    g_RCF_context.velocityReportLastTimeInMs = l_currentTimeInMs;
  }
  else
  {
    ; /* Nothing to do */
  }

  return;
}

static void RCF_displayRcfwBanner(void)
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

static void RCF_togglePrintOutput(void)
{
  if (g_RCF_printOutput == RCF_PRINT_OUTPUT_TO_CONSOLE)
  {
    LOG_info("Directing print to MASTER");

    g_RCF_printOutput = RCF_PRINT_OUTPUT_TO_MASTER;
  }
  else
  {
    LOG_info("Directing print to CONSOLE");

    g_RCF_printOutput = RCF_PRINT_OUTPUT_TO_CONSOLE;
  }

  return;
}

static void RCF_updateSwReset(void)
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

static void RCF_updateLogSetup(T_BLU_Data *p_data, uint32_t p_timeInS)
{
  switch (p_data->button)
  {
    case BLU_BUTTON_PAD_UP:
      if (g_RCF_context.padUpPressedStartTimeInS == 0)
      {
        g_RCF_context.padUpPressedStartTimeInS = p_timeInS;

        LOG_increaseLevel();
      }
      else if (p_timeInS - g_RCF_context.padUpPressedStartTimeInS < STP_BUTTONS_DEBOUNCE_PERIOD_IN_S)
      {
        ; /* Nothing to do */
      }
      else
      {
        g_RCF_context.padUpPressedStartTimeInS = 0;
      }
      break;

    case BLU_BUTTON_PAD_DOWN:
      if (g_RCF_context.padDownPressedStartTimeInS == 0)
      {
        g_RCF_context.padDownPressedStartTimeInS = p_timeInS;

        LOG_decreaseLevel();
      }
      else if (p_timeInS - g_RCF_context.padDownPressedStartTimeInS < STP_BUTTONS_DEBOUNCE_PERIOD_IN_S)
      {
        ; /* Nothing to do */
      }
      else
      {
        g_RCF_context.padDownPressedStartTimeInS = 0;
      }
      break;

    case BLU_BUTTON_PAD_LEFT:
      if (g_RCF_context.padLeftPressedStartTimeInS == 0)
      {
        g_RCF_context.padLeftPressedStartTimeInS = p_timeInS;

        LOG_toggleOnOff();
      }
      else if (p_timeInS - g_RCF_context.padLeftPressedStartTimeInS < STP_BUTTONS_DEBOUNCE_PERIOD_IN_S)
      {
        ; /* Nothing to do */
      }
      else
      {
        g_RCF_context.padLeftPressedStartTimeInS = 0;
      }
      break;

    case BLU_BUTTON_PAD_RIGHT:
      if (g_RCF_context.padRightPressedStartTimeInS == 0)
      {
        g_RCF_context.padRightPressedStartTimeInS = p_timeInS;

        RCF_togglePrintOutput();
      }
      else if (p_timeInS - g_RCF_context.padRightPressedStartTimeInS < STP_BUTTONS_DEBOUNCE_PERIOD_IN_S)
      {
        ; /* Nothing to do */
      }
      else
      {
        g_RCF_context.padRightPressedStartTimeInS = 0;
      }
      break;

    default:
      ; /* Nothing to do */;
      break;
  }

  if ((p_data->button != BLU_BUTTON_PAD_UP) && (p_data->button != BLU_BUTTON_PAD_DOWN))
  {
    g_RCF_context.padUpPressedStartTimeInS   = 0;
    g_RCF_context.padDownPressedStartTimeInS = 0;
  }
  else
  {
    ; /* Nothing to do */;
  }

  return;
}

static void RCF_updateLedMode(T_DRV_MODE p_driveMode, uint32_t p_voltageInMv)
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
