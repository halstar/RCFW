#ifndef __RCFW_H
#define __RCFW_H

#include "stm32f1xx_hal.h"

typedef enum
{
  RCF_PRINT_OUTPUT_TO_CONSOLE = 0,
  RCF_PRINT_OUTPUT_TO_MASTER,
} T_RCF_PRINT_OUTPUT;

extern T_RCF_PRINT_OUTPUT g_RCF_printOutput;

typedef struct T_RCF_Handle
{
  TIM_HandleTypeDef  *htim1 ;
  TIM_HandleTypeDef  *htim2 ;
  TIM_HandleTypeDef  *htim3 ;
  TIM_HandleTypeDef  *htim4 ;
  TIM_HandleTypeDef  *htim5 ;
  TIM_HandleTypeDef  *htim6 ;
  TIM_HandleTypeDef  *htim7 ;
  TIM_HandleTypeDef  *htim8 ;
  ADC_HandleTypeDef  *hadc1 ;
  RTC_HandleTypeDef  *hrtc  ;
  UART_HandleTypeDef *huart1;
  UART_HandleTypeDef *huart4;
} T_RCF_Handle;

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
              UART_HandleTypeDef *p_huart4);

void RCF_update(T_RCF_Handle *p_handle);

#endif /* __RCFW_H */
