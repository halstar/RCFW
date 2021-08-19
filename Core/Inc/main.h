/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define BLUE_LED_Pin GPIO_PIN_13
#define BLUE_LED_GPIO_Port GPIOC
#define SW_RESET_Pin GPIO_PIN_0
#define SW_RESET_GPIO_Port GPIOC
#define BLUETOOTH_SPI_CMD_Pin GPIO_PIN_1
#define BLUETOOTH_SPI_CMD_GPIO_Port GPIOC
#define BLUETOOTH_SPI_DAT_Pin GPIO_PIN_2
#define BLUETOOTH_SPI_DAT_GPIO_Port GPIOC
#define BLUETOOTH_SPI_CS_Pin GPIO_PIN_3
#define BLUETOOTH_SPI_CS_GPIO_Port GPIOC
#define ENCODER_REAR_RIGHT_IN_1_Pin GPIO_PIN_0
#define ENCODER_REAR_RIGHT_IN_1_GPIO_Port GPIOA
#define ENCODER_REAR_RIGHT_IN_2_Pin GPIO_PIN_1
#define ENCODER_REAR_RIGHT_IN_2_GPIO_Port GPIOA
#define BLUETOOTH_SPI_CLK_Pin GPIO_PIN_4
#define BLUETOOTH_SPI_CLK_GPIO_Port GPIOA
#define BATTERY_ADC_Pin GPIO_PIN_5
#define BATTERY_ADC_GPIO_Port GPIOA
#define ENCODER_FRONT_LEFT_IN_1_Pin GPIO_PIN_6
#define ENCODER_FRONT_LEFT_IN_1_GPIO_Port GPIOA
#define ENCODER_FRONT_LEFT_IN_2_Pin GPIO_PIN_7
#define ENCODER_FRONT_LEFT_IN_2_GPIO_Port GPIOA
#define MOTOR_REAR_LEFT_OUT_1_Pin GPIO_PIN_4
#define MOTOR_REAR_LEFT_OUT_1_GPIO_Port GPIOC
#define MOTOR_REAR_LEFT_OUT_2_Pin GPIO_PIN_5
#define MOTOR_REAR_LEFT_OUT_2_GPIO_Port GPIOC
#define MOTOR_REAR_RIGHT_OUT_1_Pin GPIO_PIN_0
#define MOTOR_REAR_RIGHT_OUT_1_GPIO_Port GPIOB
#define MOTOR_REAR_RIGHT_OUT_2_Pin GPIO_PIN_1
#define MOTOR_REAR_RIGHT_OUT_2_GPIO_Port GPIOB
#define MOTOR_REAR_RIGHT_PWM_Pin GPIO_PIN_6
#define MOTOR_REAR_RIGHT_PWM_GPIO_Port GPIOC
#define MOTOR_REAR_LEFT_PWM_Pin GPIO_PIN_7
#define MOTOR_REAR_LEFT_PWM_GPIO_Port GPIOC
#define MOTOR_FRONT_RIGHT_PWM_Pin GPIO_PIN_8
#define MOTOR_FRONT_RIGHT_PWM_GPIO_Port GPIOC
#define MOTOR_FRONT_LEFT_PWM_Pin GPIO_PIN_9
#define MOTOR_FRONT_LEFT_PWM_GPIO_Port GPIOC
#define CONSOLE_USART_TX_Pin GPIO_PIN_9
#define CONSOLE_USART_TX_GPIO_Port GPIOA
#define CONSOLE_USART_RX_Pin GPIO_PIN_10
#define CONSOLE_USART_RX_GPIO_Port GPIOA
#define ENCODER_FRONT_RIGHT_IN_1_Pin GPIO_PIN_15
#define ENCODER_FRONT_RIGHT_IN_1_GPIO_Port GPIOA
#define MASTER_USART_TX_Pin GPIO_PIN_10
#define MASTER_USART_TX_GPIO_Port GPIOC
#define MASTER_USART_RX_Pin GPIO_PIN_11
#define MASTER_USART_RX_GPIO_Port GPIOC
#define MOTOR_FRONT_RIGHT_OUT_2_Pin GPIO_PIN_12
#define MOTOR_FRONT_RIGHT_OUT_2_GPIO_Port GPIOC
#define MOTOR_FRONT_RIGHT_OUT_1_Pin GPIO_PIN_2
#define MOTOR_FRONT_RIGHT_OUT_1_GPIO_Port GPIOD
#define ENCODER_FRONT_RIGHT_IN_2_Pin GPIO_PIN_3
#define ENCODER_FRONT_RIGHT_IN_2_GPIO_Port GPIOB
#define MOTOR_FRONT_LEFT_OUT_2_Pin GPIO_PIN_4
#define MOTOR_FRONT_LEFT_OUT_2_GPIO_Port GPIOB
#define MOTOR_FRONT_LEFT_OUT_1_Pin GPIO_PIN_5
#define MOTOR_FRONT_LEFT_OUT_1_GPIO_Port GPIOB
#define ENCODER_REAR_LEFT_IN_1_Pin GPIO_PIN_6
#define ENCODER_REAR_LEFT_IN_1_GPIO_Port GPIOB
#define ENCODER_REAR_LEFT_IN_2_Pin GPIO_PIN_7
#define ENCODER_REAR_LEFT_IN_2_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
