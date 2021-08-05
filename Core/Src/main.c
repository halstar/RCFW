/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <stdio.h>
#include <string.h>

#include "log.h"
#include "led.h"
#include "utils.h"
#include "drive.h"
#include "console.h"
#include "string_fifo.h"
#include "battery_check.h"
#include "master_control.h"
#include "bluetooth_control.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

#define MAIN_MIN_BATTERY_LEVEL_IN_MV 10000
#define MAIN_LOOP_DELAY_IN_MS        10000
#define MAIN_PAD_BUTTON_PERIOD_IN_S      2

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

RTC_HandleTypeDef hrtc;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;
TIM_HandleTypeDef htim5;
TIM_HandleTypeDef htim6;
TIM_HandleTypeDef htim7;
TIM_HandleTypeDef htim8;

UART_HandleTypeDef huart4;
UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

static uint32_t g_MAIN_padUpPressedStartTime;
static uint32_t g_MAIN_padDownPressedStartTime;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_UART4_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM8_Init(void);
static void MX_RTC_Init(void);
static void MX_TIM7_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM4_Init(void);
static void MX_TIM5_Init(void);
static void MX_TIM6_Init(void);
/* USER CODE BEGIN PFP */

static void MAIN_displayRcfwBanner(void                                           );
static void MAIN_updateLogLevel   (T_BLU_Data *p_data                             );
static void MAIN_updateLedMode    (T_DRV_MODE  p_driveMode, uint32_t p_voltageInMv);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

static void MAIN_displayRcfwBanner(void)
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

static void MAIN_updateLogLevel(T_BLU_Data *p_data)
{
  RTC_TimeTypeDef l_time;
  RTC_DateTypeDef l_date;

  /* As this method is using for logging/debug, we will not deal with failure cases */
  (void)HAL_RTC_GetTime(&hrtc, &l_time, RTC_FORMAT_BCD);
  (void)HAL_RTC_GetDate(&hrtc, &l_date, RTC_FORMAT_BCD);

  switch (p_data->button)
  {
    case BLU_BUTTON_PAD_UP:
      if (g_MAIN_padUpPressedStartTime == 0)
      {
        g_MAIN_padUpPressedStartTime = UTI_turnRtcTimeToSeconds(&l_time);

        LOG_increaseLevel();
      }
      else if (UTI_turnRtcTimeToSeconds(&l_time) - g_MAIN_padUpPressedStartTime < MAIN_PAD_BUTTON_PERIOD_IN_S)
      {
        ; /* Nothing to do */
      }
      else
      {
        g_MAIN_padUpPressedStartTime = 0;
      }
      break;

    case BLU_BUTTON_PAD_DOWN:
      if (g_MAIN_padDownPressedStartTime == 0)
      {
        g_MAIN_padDownPressedStartTime = UTI_turnRtcTimeToSeconds(&l_time);

        LOG_decreaseLevel();
      }
      else if (UTI_turnRtcTimeToSeconds(&l_time) - g_MAIN_padDownPressedStartTime < MAIN_PAD_BUTTON_PERIOD_IN_S)
      {
        ; /* Nothing to do */
      }
      else
      {
        g_MAIN_padDownPressedStartTime = 0;
      }
      break;

    case BLU_BUTTON_PAD_LEFT:
      LOG_turnOff();
      break;

    case BLU_BUTTON_PAD_RIGHT:
      LOG_turnOn();
      break;

    default:
      ; /* Nothing to do */;
      break;
  }

  if ((p_data->button != BLU_BUTTON_PAD_UP) && (p_data->button != BLU_BUTTON_PAD_DOWN))
  {
    g_MAIN_padUpPressedStartTime   = 0;
    g_MAIN_padDownPressedStartTime = 0;
  }
  else
  {
    ; /* Nothing to do */;
  }

  return;
}

static void MAIN_updateLedMode(T_DRV_MODE p_driveMode, uint32_t p_voltageInMv)
{
  /* Regarding LED mode, battery check is prioritary on user requests. */
  /* Ignore 0 value as we could get it at startup or while debugging.  */
  if ((p_voltageInMv != 0) && (p_voltageInMv < MAIN_MIN_BATTERY_LEVEL_IN_MV))
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

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* RTC  clock   is used  as a calendar base (e.g. for logs)                    */
  /* TIM1 timer   is used  to update green LED              - PC13               */
  /* TIM2 to TIM5 are used as input for motor's encoders                         */
  /* TIM6 timer   is used  to produce a micro-second base for time measurement   */
  /* TIM7 timer   is used  to produce a micro-second base for utilies/delay      */
  /* TIM8 timer   is used  to produce motors' PWM    base                        */
  /* ADC1 ADC     is used  to monitor battery level         - PA5                */
  /* USART1 UART  is used  for USB/serial console           - PA9  / PA10        */
  /* UART4  UART  is used  to get control from master board - PC10 / PC11        */

  HAL_StatusTypeDef l_halReturnCode;
  T_SFO_Context     l_commandsFifo;
  T_BLU_Data        l_bluetoothData;
  T_DRV_MODE        l_driveMode;
  uint32_t          l_voltageInMv;
  uint16_t          l_lastTime;
  uint16_t          l_currentTime;
  uint16_t          l_deltaTime;

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM1_Init();
  MX_USART1_UART_Init();
  MX_UART4_Init();
  MX_ADC1_Init();
  MX_TIM8_Init();
  MX_RTC_Init();
  MX_TIM7_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_TIM5_Init();
  MX_TIM6_Init();
  /* USER CODE BEGIN 2 */

  /* Setup global variables */
  g_MAIN_padUpPressedStartTime   = 0;
  g_MAIN_padDownPressedStartTime = 0;

  /* Initialize commands string FIFO */
  SFO_init(&l_commandsFifo);

  /* Setup console */
  CON_init(&huart1);

  /* Setup master connection */
  MAS_init(&huart4);

  /* Setup and start using logs */
  LOG_init    (&hrtc          );
  LOG_setLevel(LOG_LEVEL_INFO);
  LOG_turnOn  (               );
  LOG_info    ("Starting RCFW");

  /* Display RCFW banner */
  MAIN_displayRcfwBanner();

  /* Initialize Timer 6 */
  l_halReturnCode = HAL_TIM_Base_Start(&htim6);

  if (l_halReturnCode != HAL_OK)
  {
    LOG_error("HAL_TIM_Base_Start(&htim6) returned an error code: %d", l_halReturnCode);
  }
  else
  {
    LOG_info("Started TIMER 6 (time measurement)");
  }

  /* Initialize Timer 7 and delay function in utilities */
  UTI_init(&htim7);

  l_halReturnCode = HAL_TIM_Base_Start_IT(&htim7);

  if (l_halReturnCode != HAL_OK)
  {
    LOG_error("HAL_TIM_Base_Start_IT(&htim7) returned an error code: %d", l_halReturnCode);
  }
  else
  {
    LOG_info("Started TIMER 7 (utilities/delay)");
  }

  /* Initialize Timer 1 & green LED */
  LED_setMode(LED_MODE_BLINK_FAST);

  l_halReturnCode = HAL_TIM_Base_Start_IT(&htim1);

  if (l_halReturnCode != HAL_OK)
  {
    LOG_error("HAL_TIM_Base_Start_IT(&htim1) returned an error code: %d", l_halReturnCode);
  }
  else
  {
    LOG_info("Started TIMER 1 (green LED)");
  }

  /* Initialize Timers 2, 3, 4 & 5 */
  l_halReturnCode  = HAL_TIM_Encoder_Start_IT(&htim2, TIM_CHANNEL_ALL);
  l_halReturnCode |= HAL_TIM_Encoder_Start_IT(&htim3, TIM_CHANNEL_ALL);
  l_halReturnCode |= HAL_TIM_Encoder_Start_IT(&htim4, TIM_CHANNEL_ALL);
  l_halReturnCode |= HAL_TIM_Encoder_Start_IT(&htim5, TIM_CHANNEL_ALL);

  if (l_halReturnCode != HAL_OK)
  {
    LOG_error("HAL_TIM_Encoder_Start_IT(&htimX) returned error code(s): %d", l_halReturnCode);
  }
  else
  {
    LOG_info("Started TIMER 2, 3, 4, 5 (encoders)");
  }

  /* Initialize Timer 8 */
  l_halReturnCode = HAL_TIM_Base_Start(&htim8);

  if (l_halReturnCode != HAL_OK)
  {
    LOG_error("HAL_TIM_Base_Start(&htim8) returned an error code: %d", l_halReturnCode);
  }
  else
  {
    LOG_info("Started TIMER 8 (PWM channels)");
  }

  /* Initialize battery monitor */
  BAT_init(&hadc1, &hrtc);

  /* Initialize PWM channels */
  l_halReturnCode  = HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_1);
  l_halReturnCode |= HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_2);
  l_halReturnCode |= HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_3);
  l_halReturnCode |= HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_4);

  if (l_halReturnCode != HAL_OK)
  {
    LOG_error("HAL_TIM_PWM_Start(&htim8) returned error code(s): %d", l_halReturnCode);
  }
  else
  {
    LOG_info("Started PWM channels");
  }

  /* Initialize bluetooth control */
  BLU_init(DRV_MAXIMUM_SPEED);

  /* Initialize driving module */
  DRV_init(&htim8, &htim2, &htim3, &htim4, &htim5);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  /* Initialize time measurement for master board control */
  l_lastTime = __HAL_TIM_GET_COUNTER(&htim6);

  while (1)
  {
    BLU_receiveData        (&l_bluetoothData);
    DRV_updateFromBluetooth(&l_bluetoothData);

    l_driveMode = DRV_getMode();

    BAT_update         (&l_voltageInMv            );
    MAIN_updateLedMode (l_driveMode, l_voltageInMv);
    MAIN_updateLogLevel(&l_bluetoothData          );

    UTI_delayUs(MAIN_LOOP_DELAY_IN_MS);

    CON_updateFifo(&l_commandsFifo);
    MAS_updateFifo(&l_commandsFifo);

    l_currentTime = __HAL_TIM_GET_COUNTER(&htim6);
    l_deltaTime   = l_lastTime - l_currentTime;
    l_lastTime    = l_currentTime;

    DRV_updateFromMaster(&l_commandsFifo, l_deltaTime);

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_ADC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */
  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_5;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef DateToUpdate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */
  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
  hrtc.Init.OutPut = RTC_OUTPUTSOURCE_NONE;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0x0;
  sTime.Minutes = 0x0;
  sTime.Seconds = 0x0;

  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  DateToUpdate.WeekDay = RTC_WEEKDAY_MONDAY;
  DateToUpdate.Month = RTC_MONTH_JANUARY;
  DateToUpdate.Date = 0x1;
  DateToUpdate.Year = 0x0;

  if (HAL_RTC_SetDate(&hrtc, &DateToUpdate, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 7999;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 249;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_Encoder_InitTypeDef sConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 65535;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  sConfig.EncoderMode = TIM_ENCODERMODE_TI12;
  sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC1Filter = 10;
  sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC2Filter = 0;
  if (HAL_TIM_Encoder_Init(&htim2, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_Encoder_InitTypeDef sConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 0;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 65535;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  sConfig.EncoderMode = TIM_ENCODERMODE_TI12;
  sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC1Filter = 10;
  sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC2Filter = 0;
  if (HAL_TIM_Encoder_Init(&htim3, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_Encoder_InitTypeDef sConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 0;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 65535;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  sConfig.EncoderMode = TIM_ENCODERMODE_TI12;
  sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC1Filter = 10;
  sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC2Filter = 0;
  if (HAL_TIM_Encoder_Init(&htim4, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */

}

/**
  * @brief TIM5 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM5_Init(void)
{

  /* USER CODE BEGIN TIM5_Init 0 */

  /* USER CODE END TIM5_Init 0 */

  TIM_Encoder_InitTypeDef sConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM5_Init 1 */

  /* USER CODE END TIM5_Init 1 */
  htim5.Instance = TIM5;
  htim5.Init.Prescaler = 0;
  htim5.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim5.Init.Period = 65535;
  htim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim5.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  sConfig.EncoderMode = TIM_ENCODERMODE_TI12;
  sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC1Filter = 10;
  sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC2Filter = 0;
  if (HAL_TIM_Encoder_Init(&htim5, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim5, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM5_Init 2 */

  /* USER CODE END TIM5_Init 2 */

}

/**
  * @brief TIM6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM6_Init(void)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 7;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 65535;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM6_Init 2 */

  /* USER CODE END TIM6_Init 2 */

}

/**
  * @brief TIM7 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM7_Init(void)
{

  /* USER CODE BEGIN TIM7_Init 0 */

  /* USER CODE END TIM7_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM7_Init 1 */

  /* USER CODE END TIM7_Init 1 */
  htim7.Instance = TIM7;
  htim7.Init.Prescaler = 7;
  htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim7.Init.Period = 65535;
  htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM7_Init 2 */

  /* USER CODE END TIM7_Init 2 */

}

/**
  * @brief TIM8 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM8_Init(void)
{

  /* USER CODE BEGIN TIM8_Init 0 */

  /* USER CODE END TIM8_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM8_Init 1 */

  /* USER CODE END TIM8_Init 1 */
  htim8.Instance = TIM8;
  htim8.Init.Prescaler = 7;
  htim8.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim8.Init.Period = 100;
  htim8.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim8.Init.RepetitionCounter = 0;
  htim8.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim8) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim8, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim8) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim8, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim8, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim8, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim8, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim8, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim8, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM8_Init 2 */

  /* USER CODE END TIM8_Init 2 */
  HAL_TIM_MspPostInit(&htim8);

}

/**
  * @brief UART4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART4_Init(void)
{

  /* USER CODE BEGIN UART4_Init 0 */

  /* USER CODE END UART4_Init 0 */

  /* USER CODE BEGIN UART4_Init 1 */

  /* USER CODE END UART4_Init 1 */
  huart4.Instance = UART4;
  huart4.Init.BaudRate = 9600;
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART4_Init 2 */

  /* USER CODE END UART4_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, BLUE_LED_Pin|BLUETOOTH_SPI_CMD_Pin|BLUETOOTH_SPI_CS_Pin|MOTOR_FRONT_RIGHT_IN_1_Pin
                          |MOTOR_FRONT_RIGHT_IN_2_Pin|MOTOR_REAR_LEFT_IN_2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(BLUETOOTH_SPI_CLK_GPIO_Port, BLUETOOTH_SPI_CLK_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, MOTOR_FRONT_LEFT_IN_1_Pin|MOTOR_FRONT_LEFT_IN_2_Pin|MOTOR_REAR_RIGHT_IN_2_Pin|MOTOR_REAR_RIGHT_IN_1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(MOTOR_REAR_LEFT_IN_1_GPIO_Port, MOTOR_REAR_LEFT_IN_1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : BLUE_LED_Pin */
  GPIO_InitStruct.Pin = BLUE_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(BLUE_LED_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : BLUETOOTH_SPI_CMD_Pin BLUETOOTH_SPI_CS_Pin MOTOR_FRONT_RIGHT_IN_1_Pin MOTOR_FRONT_RIGHT_IN_2_Pin
                           MOTOR_REAR_LEFT_IN_2_Pin */
  GPIO_InitStruct.Pin = BLUETOOTH_SPI_CMD_Pin|BLUETOOTH_SPI_CS_Pin|MOTOR_FRONT_RIGHT_IN_1_Pin|MOTOR_FRONT_RIGHT_IN_2_Pin
                          |MOTOR_REAR_LEFT_IN_2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : BLUETOOTH_SPI_DAT_Pin */
  GPIO_InitStruct.Pin = BLUETOOTH_SPI_DAT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(BLUETOOTH_SPI_DAT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : BLUETOOTH_SPI_CLK_Pin */
  GPIO_InitStruct.Pin = BLUETOOTH_SPI_CLK_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(BLUETOOTH_SPI_CLK_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : MOTOR_FRONT_LEFT_IN_1_Pin MOTOR_FRONT_LEFT_IN_2_Pin MOTOR_REAR_RIGHT_IN_2_Pin MOTOR_REAR_RIGHT_IN_1_Pin */
  GPIO_InitStruct.Pin = MOTOR_FRONT_LEFT_IN_1_Pin|MOTOR_FRONT_LEFT_IN_2_Pin|MOTOR_REAR_RIGHT_IN_2_Pin|MOTOR_REAR_RIGHT_IN_1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : MOTOR_REAR_LEFT_IN_1_Pin */
  GPIO_InitStruct.Pin = MOTOR_REAR_LEFT_IN_1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(MOTOR_REAR_LEFT_IN_1_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
  if ((htim == &htim2) || (htim == &htim3) || (htim == &htim4) || (htim == &htim5))
  {
    DRV_updateEncoder(htim);
  }
  else
  {
    ; /* Nothing to do */
  }

  return;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* Check the handle of the timer triggering this callback and update LED */
  if (htim == &htim1)
  {
    LED_update();
  }
  else
  {
    ; /* Nothing to do */
  }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  /* Check the handle of the UART triggering this callback and actually receive date */
  if (huart == &huart1)
  {
    CON_receiveData();
  }
  else if (huart == &huart4)
  {
    MAS_receiveData();
  }
  else
  {
    ; /* Nothing to do */
  }
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();

  LOG_error("Error_Handler() triggered");

  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
     LOG_error("Wrong parameters: %s@%d", file, line);

    return;
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
