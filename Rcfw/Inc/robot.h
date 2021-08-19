#ifndef __ROBOT_H
#define __ROBOT_H

#include "stm32f1xx_hal.h"

typedef enum
{
  RBT_PRINT_OUTPUT_TO_CONSOLE = 0,
  RBT_PRINT_OUTPUT_TO_MASTER,
} T_RBT_PRINT_OUTPUT;

extern T_RBT_PRINT_OUTPUT g_RBT_printOutput;

typedef struct T_RBT_Handle
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
} T_RBT_Handle;

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
              UART_HandleTypeDef *p_huart4);

void RBT_update(T_RBT_Handle *p_handle);

#endif /* __ROBOT_H */
