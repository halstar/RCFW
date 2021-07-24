#include <stdbool.h>

#include "bluetooth_control.h"

#include "stm32f1xx_hal.h"
#include "utils.h"
#include "main.h"
#include "log.h"

#define BLUETOOTH_CONTROL_DATA_BUFFER_LENGTH 7

#define BLUETOOTH_CONTROL_UNUSED_DATA_OFFSET 0
#define BLUETOOTH_CONTROL_BUTTON_LOW_OFFSET  1
#define BLUETOOTH_CONTROL_BUTTON_HIGH_OFFSET 2
#define BLUETOOTH_CONTROL_RIGHT_X_OFFSET     3
#define BLUETOOTH_CONTROL_RIGHT_Y_OFFSET     4
#define BLUETOOTH_CONTROL_LEFT_X_OFFSET      5
#define BLUETOOTH_CONTROL_LEFT_Y_OFFSET      6

static uint32_t BLUETOOTH_CONTROL_maxDataValue;

static T_BLUETOOTH_CONTROL_Data BLUETOOTH_CONTROL_DATA_lastData = { .leftX = 128, .leftY = 128, .rightX = 128, .rightY = 128, .button = BLUETOOTH_CONTROL_BUTTON_NONE};

static void                       BLUETOOTH_CONTROL_sendCommand  (uint8_t  p_command                                               );
static void                       BLUETOOTH_CONTROL_readData     (uint8_t *l_buffer                                                );
static T_BLUETOOTH_CONTROL_BUTTON BLUETOOTH_CONTROL_getButton    (uint8_t *l_buffer                                                );
static int32_t                    BLUETOOTH_CONTROL_normalizeData(uint32_t p_rawData, uint32_t p_maxValue, bool p_isInversionNeeded);

static void BLUETOOTH_CONTROL_sendCommand(uint8_t p_command)
{
  volatile uint16_t l_bitValue;

  for (l_bitValue = 0x01; l_bitValue < 0x0100; l_bitValue <<= 1)
  {
    if ((l_bitValue & p_command) != 0)
    {
      SET_BIT(GPIOC->ODR, BLUETOOTH_SPI_CMD_Pin);
    }
    else
    {
      CLEAR_BIT(GPIOC->ODR, BLUETOOTH_SPI_CMD_Pin);
    }

    SET_BIT(GPIOA->ODR  , BLUETOOTH_SPI_CLK_Pin);
    UTILS_delayUs(5);
    CLEAR_BIT(GPIOA->ODR, BLUETOOTH_SPI_CLK_Pin);
    UTILS_delayUs(5);
    SET_BIT(GPIOA->ODR  , BLUETOOTH_SPI_CLK_Pin);
  }
  UTILS_delayUs(16);

  return;
}

static void BLUETOOTH_CONTROL_readData(uint8_t *l_buffer)
{
  volatile uint8_t  l_index;
  volatile uint16_t l_bitValue;

  /* Reset buffer */
  for (l_index = 0; l_index < BLUETOOTH_CONTROL_DATA_BUFFER_LENGTH; l_index++)
  {
    l_buffer[l_index] = 0x00;
  }

  CLEAR_BIT(GPIOC->ODR, BLUETOOTH_SPI_CS_Pin);

  BLUETOOTH_CONTROL_sendCommand(0x01);
  BLUETOOTH_CONTROL_sendCommand(0x42);

  for (l_index = 0; l_index < BLUETOOTH_CONTROL_DATA_BUFFER_LENGTH; l_index++)
  {
    for (l_bitValue = 0x01; l_bitValue < 0x100; l_bitValue <<= 1)
    {
      SET_BIT(GPIOA->ODR  , BLUETOOTH_SPI_CLK_Pin);
      UTILS_delayUs(5);
      CLEAR_BIT(GPIOA->ODR, BLUETOOTH_SPI_CLK_Pin);
      UTILS_delayUs(5);
      SET_BIT(GPIOA->ODR  , BLUETOOTH_SPI_CLK_Pin);

      if (READ_BIT(GPIOC->IDR, BLUETOOTH_SPI_DAT_Pin) != GPIO_PIN_RESET)
      {
        l_buffer[l_index] |= l_bitValue;
      }
      else
      {
        ; /* Nothing to do */
      }
    }
      UTILS_delayUs(16);
  }

  SET_BIT(GPIOC->ODR, BLUETOOTH_SPI_CS_Pin);

  return;
}

static T_BLUETOOTH_CONTROL_BUTTON BLUETOOTH_CONTROL_getButton(uint8_t *l_buffer)
{
  uint16_t l_buttonRawValue;
  uint8_t  l_index;

  l_buttonRawValue = (l_buffer[BLUETOOTH_CONTROL_BUTTON_HIGH_OFFSET] << 8) | l_buffer[BLUETOOTH_CONTROL_BUTTON_LOW_OFFSET];

  /* Ignore BLUETOOTH_CONTROL_BUTTON_NONE value as it is not directly coded in raw data */
  for (l_index = BLUETOOTH_CONTROL_BUTTON_SELECT; l_index < BLUETOOTH_CONTROL_BUTTON_COUNT_VALUE; l_index++)
  {
    /* Stop decoding button data on 1st match (do not deal with multiple presses case) */
    if ((l_buttonRawValue & (1 << (l_index - 1))) == 0)
    {
      return l_index;
    }
    else
    {
      ; /* Nothing to do */
    }
  }

  return BLUETOOTH_CONTROL_BUTTON_NONE;
}

static int32_t BLUETOOTH_CONTROL_normalizeData(uint32_t p_rawData, uint32_t p_maxValue, bool p_isInversionNeeded)
{
  float l_normalizedData;

  l_normalizedData = p_rawData - 128.0f;

  if (l_normalizedData > 0.0f)
  {
    l_normalizedData *= (float)p_maxValue / 127.0f;
  }
  else
  {
    l_normalizedData *= (float)p_maxValue / 128.0f;
  }

  if (p_isInversionNeeded == true)
  {
    l_normalizedData *= -1.0f;
  }
  else
  {
    ; /* Nothing to do */
  }

  return (int32_t)l_normalizedData;
}

void BLUETOOTH_CONTROL_init(uint32_t p_maxDataValue)
{
  LOG_info("Initializing bluetooth control");

  BLUETOOTH_CONTROL_maxDataValue = p_maxDataValue;

  return;
}


void BLUETOOTH_CONTROL_receiveData(T_BLUETOOTH_CONTROL_Data *p_data)
{
  uint8_t                    l_buffer[BLUETOOTH_CONTROL_DATA_BUFFER_LENGTH];
  uint32_t                   l_leftX;
  uint32_t                   l_leftY;
  uint32_t                   l_rightX;
  uint32_t                   l_rightY;
  T_BLUETOOTH_CONTROL_BUTTON l_button;

  // LOG_info("Receiving Bluetooth data");

  /* Read raw data */
  BLUETOOTH_CONTROL_readData(l_buffer);

  /* Start and decode raw data */
  l_leftX  = l_buffer[BLUETOOTH_CONTROL_LEFT_X_OFFSET ];
  l_leftY  = l_buffer[BLUETOOTH_CONTROL_LEFT_Y_OFFSET ];
  l_rightX = l_buffer[BLUETOOTH_CONTROL_RIGHT_X_OFFSET];
  l_rightY = l_buffer[BLUETOOTH_CONTROL_RIGHT_Y_OFFSET];
  l_button = BLUETOOTH_CONTROL_getButton(l_buffer);

  /* Deal with startup condition, while read data is not valid yet */
  if  ((l_leftX == 255 && l_leftY == 255 && l_rightX == 255 && l_rightY ==255)
    || (l_leftX ==   0 && l_leftY ==   0 && l_rightX ==   0 && l_rightY ==  0))
  {
    l_leftX  = 128;
    l_leftY  = 128;
    l_rightX = 128;
    l_rightY = 128;
    l_button = BLUETOOTH_CONTROL_BUTTON_NONE;
  }
  /* Use a confirmation mechanism, on 2 cycles, as glitches are observed */
  else if ((l_leftX  == BLUETOOTH_CONTROL_DATA_lastData.leftX) &&
           (l_leftY  == BLUETOOTH_CONTROL_DATA_lastData.leftY) &&
           (l_rightX == BLUETOOTH_CONTROL_DATA_lastData.rightX) &&
           (l_rightY == BLUETOOTH_CONTROL_DATA_lastData.rightY) &&
           (l_button == BLUETOOTH_CONTROL_DATA_lastData.button))
  {
    /* Normalize directions data in range [-100..100] */
    p_data->leftX  = BLUETOOTH_CONTROL_normalizeData(l_leftX , BLUETOOTH_CONTROL_maxDataValue, false);
    p_data->leftY  = BLUETOOTH_CONTROL_normalizeData(l_leftY , BLUETOOTH_CONTROL_maxDataValue, true );
    p_data->rightX = BLUETOOTH_CONTROL_normalizeData(l_rightX, BLUETOOTH_CONTROL_maxDataValue, false);
    p_data->rightY = BLUETOOTH_CONTROL_normalizeData(l_rightY, BLUETOOTH_CONTROL_maxDataValue, true );
    p_data->button = l_button;
  }
  else
  {
    ; /* Nothing to do */
  }

  /* Saved received data for later use in confirmation mechanism */
  BLUETOOTH_CONTROL_DATA_lastData.leftX  = l_leftX;
  BLUETOOTH_CONTROL_DATA_lastData.leftY  = l_leftY;
  BLUETOOTH_CONTROL_DATA_lastData.rightX = l_rightX;
  BLUETOOTH_CONTROL_DATA_lastData.rightY = l_rightY;
  BLUETOOTH_CONTROL_DATA_lastData.button = l_button;

  return;
}
