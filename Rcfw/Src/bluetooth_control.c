#include <stdbool.h>

#include "bluetooth_control.h"

#include "stm32f1xx_hal.h"
#include "utils.h"
#include "setup.h"
#include "main.h"
#include "log.h"

#define BLU_DATA_BUFFER_LENGTH      7
#define BLU_DATA_DEFAUT_DIRECTION 128

#define BLU_UNUSED_DATA_OFFSET 0
#define BLU_BUTTON_LOW_OFFSET  1
#define BLU_BUTTON_HIGH_OFFSET 2
#define BLU_RIGHT_X_OFFSET     3
#define BLU_RIGHT_Y_OFFSET     4
#define BLU_LEFT_X_OFFSET      5
#define BLU_LEFT_Y_OFFSET      6

typedef struct T_BLU_Context
{
  T_BLU_Data lastData;
} T_BLU_Context;

static T_BLU_Context g_BLU_context;

static void         BLU_sendCommand(uint32_t  p_command);
static void         BLU_readData   (uint8_t  *p_buffer );
static T_BLU_BUTTON BLU_getButton  (uint8_t  *p_buffer );

static void BLU_sendCommand(uint32_t p_command)
{
  uint32_t l_bitValue;

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
    UTI_delayUs(5);
    CLEAR_BIT(GPIOA->ODR, BLUETOOTH_SPI_CLK_Pin);
    UTI_delayUs(5);
    SET_BIT(GPIOA->ODR  , BLUETOOTH_SPI_CLK_Pin);
  }
  UTI_delayUs(16);

  return;
}

static void BLU_readData(uint8_t *p_buffer)
{
  uint32_t l_index;
  uint32_t l_bitValue;

  /* Reset buffer */
  for (l_index = 0; l_index < BLU_DATA_BUFFER_LENGTH; l_index++)
  {
    p_buffer[l_index] = 0x00;
  }

  CLEAR_BIT(GPIOC->ODR, BLUETOOTH_SPI_CS_Pin);

  BLU_sendCommand(0x01);
  BLU_sendCommand(0x42);

  for (l_index = 0; l_index < BLU_DATA_BUFFER_LENGTH; l_index++)
  {
    for (l_bitValue = 0x01; l_bitValue < 0x100; l_bitValue <<= 1)
    {
      SET_BIT(GPIOA->ODR  , BLUETOOTH_SPI_CLK_Pin);
      UTI_delayUs(5);
      CLEAR_BIT(GPIOA->ODR, BLUETOOTH_SPI_CLK_Pin);
      UTI_delayUs(5);
      SET_BIT(GPIOA->ODR  , BLUETOOTH_SPI_CLK_Pin);

      if (READ_BIT(GPIOC->IDR, BLUETOOTH_SPI_DAT_Pin) != GPIO_PIN_RESET)
      {
        p_buffer[l_index] |= l_bitValue;
      }
      else
      {
        ; /* Nothing to do */
      }
    }
      UTI_delayUs(16);
  }

  SET_BIT(GPIOC->ODR, BLUETOOTH_SPI_CS_Pin);

  return;
}

static T_BLU_BUTTON BLU_getButton(uint8_t *p_buffer)
{
  uint32_t l_buttonRawValue;
  uint32_t l_index;

  l_buttonRawValue = (p_buffer[BLU_BUTTON_HIGH_OFFSET] << 8) | p_buffer[BLU_BUTTON_LOW_OFFSET];

  /* Ignore BLU_BUTTON_NONE value as it is not directly coded in raw data */
  for (l_index = BLU_BUTTON_SELECT; l_index < BLU_BUTTON_COUNT_VALUE; l_index++)
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

  return BLU_BUTTON_NONE;
}

void BLU_init(void)
{
  LOG_info("Initializing bluetooth control");

  BLU_initData(&g_BLU_context.lastData);

  return;
}

void BLU_initData(T_BLU_Data *p_data)
{
  p_data->leftX  = BLU_DATA_DEFAUT_DIRECTION;
  p_data->leftY  = BLU_DATA_DEFAUT_DIRECTION;
  p_data->rightX = BLU_DATA_DEFAUT_DIRECTION;
  p_data->rightY = BLU_DATA_DEFAUT_DIRECTION;
  p_data->button = BLU_BUTTON_NONE;

  return;
}

void BLU_receiveData(T_BLU_Data *p_data)
{
  uint8_t    l_buffer[BLU_DATA_BUFFER_LENGTH];
  T_BLU_Data l_readData;

  /* Read raw data */
  BLU_readData(l_buffer);

  /* Start and decode raw data */
  l_readData.leftX  = l_buffer[BLU_LEFT_X_OFFSET ];
  l_readData.leftY  = l_buffer[BLU_LEFT_Y_OFFSET ];
  l_readData.rightX = l_buffer[BLU_RIGHT_X_OFFSET];
  l_readData.rightY = l_buffer[BLU_RIGHT_Y_OFFSET];
  l_readData.button = BLU_getButton(l_buffer);

  /* Deal with startup condition, while read data is not valid yet */
  if  ((l_readData.leftX == 255 && l_readData.leftY == 255 && l_readData.rightX == 255 && l_readData.rightY ==255)
    || (l_readData.leftX ==   0 && l_readData.leftY ==   0 && l_readData.rightX ==   0 && l_readData.rightY ==  0))
  {
    BLU_initData(&l_readData);
  }
  /* Use a confirmation mechanism, on 2 cycles, as glitches are observed */
  else if ((l_readData.leftX  == g_BLU_context.lastData.leftX) &&
           (l_readData.leftY  == g_BLU_context.lastData.leftY) &&
           (l_readData.rightX == g_BLU_context.lastData.rightX) &&
           (l_readData.rightY == g_BLU_context.lastData.rightY) &&
           (l_readData.button == g_BLU_context.lastData.button))
  {
    /* Normalize directions data in range [-MAX..MAX] */
    p_data->leftX  = UTI_normalizeIntValue(l_readData.leftX , 0, 255, -STP_DRIVE_MAX_SPEED, STP_DRIVE_MAX_SPEED, false);
    p_data->leftY  = UTI_normalizeIntValue(l_readData.leftY , 0, 255, -STP_DRIVE_MAX_SPEED, STP_DRIVE_MAX_SPEED, true );
    p_data->rightX = UTI_normalizeIntValue(l_readData.rightX, 0, 255, -STP_DRIVE_MAX_SPEED, STP_DRIVE_MAX_SPEED, false);
    p_data->rightY = UTI_normalizeIntValue(l_readData.rightY, 0, 255, -STP_DRIVE_MAX_SPEED, STP_DRIVE_MAX_SPEED, true );
    p_data->button = l_readData.button;
  }
  else
  {
    ; /* Nothing to do */
  }

  /* Saved received data for later use in confirmation mechanism */
  g_BLU_context.lastData.leftX  = l_readData.leftX;
  g_BLU_context.lastData.leftY  = l_readData.leftY;
  g_BLU_context.lastData.rightX = l_readData.rightX;
  g_BLU_context.lastData.rightY = l_readData.rightY;
  g_BLU_context.lastData.button = l_readData.button;

  return;
}
