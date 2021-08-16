#include "battery_check.h"

#include "log.h"

typedef struct T_ADC_Context
{
  ADC_HandleTypeDef *adcHandle;
} T_ADC_Context;

static T_ADC_Context g_ADC_context;

void BAT_init(ADC_HandleTypeDef *p_adcHandle)
{
  LOG_info("Initializing battery check");

  g_ADC_context.adcHandle = p_adcHandle;

  return;
}

void BAT_update(uint32_t *p_voltageInMv)
{
  uint32_t l_adcRawData;
  float    l_voltageInV;

  l_adcRawData = HAL_ADC_GetValue(g_ADC_context.adcHandle);

  /* Apply conversion based on STM32 reference voltage & resolution */
  l_voltageInV  = (l_adcRawData * 3.30f ) / 4096.0f;

  /* Consider voltage divider used between the battery & the ADC input */
  l_voltageInV *= 11.0f;

  *p_voltageInMv = (uint32_t)(l_voltageInV * 1000.0f);

  LOG_debug("Battery level: %u mV", *p_voltageInMv);

  return;
}
