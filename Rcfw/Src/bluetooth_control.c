#include <stdbool.h>

#include "bluetooth_control.h"

#include "stm32f1xx_hal.h"
#include "utils.h"
#include "log.h"

#define BITBAND(addr, bitnum) ((addr & 0xF0000000)+0x2000000+((addr &0xFFFFF)<<5)+(bitnum<<2))
#define MEM_ADDR(addr)  *((volatile unsigned long  *)(addr))
#define BIT_ADDR(addr, bitnum)   MEM_ADDR(BITBAND(addr, bitnum))

#define GPIOA_ODR_Addr    (GPIOA_BASE+12)
#define GPIOC_ODR_Addr    (GPIOC_BASE+12)
#define GPIOC_IDR_Addr    (GPIOC_BASE+8)

#define PAout(n)   BIT_ADDR(GPIOA_ODR_Addr,n)
#define PCout(n)   BIT_ADDR(GPIOC_ODR_Addr,n)
#define PCin(n)    BIT_ADDR(GPIOC_IDR_Addr,n)

#define DI   PCin(2)

#define DO_H PCout(1)=1
#define DO_L PCout(1)=0

#define CS_H PCout(3)=1
#define CS_L PCout(3)=0

#define CLK_H PAout(4)=1
#define CLK_L PAout(4)=0

#define BLUETOOTH_CONTROL_RIGHT_X_OFFSET 5
#define BLUETOOTH_CONTROL_RIGHT_Y_OFFSET 6
#define BLUETOOTH_CONTROL_LEFT_X_OFFSET  7
#define BLUETOOTH_CONTROL_LEFT_Y_OFFSET  8

static uint8_t  BLUETOOTH_CONTROL_buffer[9] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint16_t BLUETOOTH_CONTROL_mask   [] =
{
  BUTTON_SELECT,
  BUTTON_L3,
  BUTTON_R3 ,
  BUTTON_START,
  BUTTON_PAD_UP,
  BUTTON_PAD_RIGHT,
  BUTTON_PAD_DOWN,
  BUTTON_PAD_LEFT,
  BUTTON_L2,
  BUTTON_R2,
  BUTTON_L1,
  BUTTON_R1 ,
  BUTTON_GREEN_TRIANGLE,
  BUTTON_RED_CIRCLE,
  BUTTON_BLUE_CROSS,
  BUTTON_PINK_SQUARE
};
T_BLUETOOTH_ControlData BLUETOOTH_CONTROL_DATA_lastData = { .leftX = 128, .leftY = 128, .rightX = 128, .rightY = 128, .button = BUTTON_NONE};

static void    BLUETOOTH_CONTROL_readData(void);
static void    BLUETOOTH_CONTROL_sendCommand(uint8_t command);
static uint8_t BLUETOOTH_CONTROL_getButtonData(void);
static void    BLUETOOTH_CONTROL_clearData(void);
static int32_t BLUETOOTH_CONTROL_normalizeData(uint32_t rawData, bool isInversionNeeded);

static void BLUETOOTH_CONTROL_sendCommand(uint8_t command)
{
  volatile uint16_t ref=0x01;
  BLUETOOTH_CONTROL_buffer[1] = 0;
  for(ref=0x01;ref<0x0100;ref<<=1)
  {
    if(ref&command)
    {
      DO_H;
    }
    else DO_L;

    CLK_H;
    UTILS_delayUs(5);
    CLK_L;
    UTILS_delayUs(5);
    CLK_H;
    if(DI)
      BLUETOOTH_CONTROL_buffer[1] = ref|BLUETOOTH_CONTROL_buffer[1];
  }
  UTILS_delayUs(16);

  return;
}

static void BLUETOOTH_CONTROL_readData(void)
{
  volatile uint8_t byte=0;
  volatile uint16_t ref=0x01;
  CS_L;
  BLUETOOTH_CONTROL_sendCommand(0x01);
  BLUETOOTH_CONTROL_sendCommand(0x42);
  for(byte=2;byte<9;byte++)
  {
    for(ref=0x01;ref<0x100;ref<<=1)
    {
      CLK_H;
      UTILS_delayUs(5);
      CLK_L;
      UTILS_delayUs(5);
      CLK_H;
          if(DI)
          BLUETOOTH_CONTROL_buffer[byte] = ref|BLUETOOTH_CONTROL_buffer[byte];
    }
        UTILS_delayUs(16);
  }
  CS_H;

  return;
}

static uint8_t BLUETOOTH_CONTROL_getButtonData()
{
  uint8_t index;
  uint16_t button;

  BLUETOOTH_CONTROL_clearData();
  BLUETOOTH_CONTROL_readData();
  button=(BLUETOOTH_CONTROL_buffer[4]<<8)|BLUETOOTH_CONTROL_buffer[3];
  for(index=0;index<16;index++)
  {
    if((button&(1<<(BLUETOOTH_CONTROL_mask[index]-1)))==0)
    return index+1;
  }

  return 0;
}

static void BLUETOOTH_CONTROL_clearData()
{
  uint8_t a;
  for(a=0;a<9;a++)
    BLUETOOTH_CONTROL_buffer[a]=0x00;

  return;
}

static int32_t BLUETOOTH_CONTROL_normalizeData(uint32_t rawData, bool isInversionNeeded)
{
  float normalizedData;

  normalizedData = rawData - 128.0f;

  if (normalizedData > 0.0f)
  {
    normalizedData *= 100.0f / 127.0f;
  }
  else
  {
    normalizedData *= 100.0f / 128.0f;
  }

  if (isInversionNeeded == true)
  {
    normalizedData *= -1.0f;
  }

  return (int32_t)normalizedData;
}

void BLUETOOTH_CONTROL_receiveData(T_BLUETOOTH_ControlData *data)
{
  uint32_t leftX;
  uint32_t leftY;
  uint32_t rightX;
  uint32_t rightY;
  uint32_t button;

  // LOG_info("Receiving Bluetooth data");

  /* Read raw data */
  leftX  = BLUETOOTH_CONTROL_buffer[BLUETOOTH_CONTROL_LEFT_X_OFFSET ];
  leftY  = BLUETOOTH_CONTROL_buffer[BLUETOOTH_CONTROL_LEFT_Y_OFFSET ];
  rightX = BLUETOOTH_CONTROL_buffer[BLUETOOTH_CONTROL_RIGHT_X_OFFSET];
  rightY = BLUETOOTH_CONTROL_buffer[BLUETOOTH_CONTROL_RIGHT_Y_OFFSET];
  button = BLUETOOTH_CONTROL_getButtonData();

  /* Deal with startup condition, while read data is not valid yet */
  if  ((leftX == 255 && leftY == 255 && rightX == 255 && rightY ==255)
    || (leftX ==   0 && leftY ==   0 && rightX ==   0 && rightY ==  0))
  {
    leftX  = 128;
    leftY  = 128;
    rightX = 128;
    rightY = 128;
    button = BUTTON_NONE;
  }
  /* Use a confirmation mechanism, on 2 cycles, as glitches are observed */
  else if ((leftX  == BLUETOOTH_CONTROL_DATA_lastData.leftX) &&
           (leftY  == BLUETOOTH_CONTROL_DATA_lastData.leftY) &&
           (rightX == BLUETOOTH_CONTROL_DATA_lastData.rightX) &&
           (rightY == BLUETOOTH_CONTROL_DATA_lastData.rightY) &&
           (button == BLUETOOTH_CONTROL_DATA_lastData.button))
  {
    /* Normalize directions data in range [-100..100] */
    data->leftX  = BLUETOOTH_CONTROL_normalizeData(leftX , false);
    data->leftY  = BLUETOOTH_CONTROL_normalizeData(leftY , true );
    data->rightX = BLUETOOTH_CONTROL_normalizeData(rightX, false);
    data->rightY = BLUETOOTH_CONTROL_normalizeData(rightY, true );
    data->button = button;
  }
  else
  {
    ; /* Nothing to do */
  }

  /* Saved received data for later use in confirmation mechanism */
  BLUETOOTH_CONTROL_DATA_lastData.leftX  = leftX;
  BLUETOOTH_CONTROL_DATA_lastData.leftY  = leftY;
  BLUETOOTH_CONTROL_DATA_lastData.rightX = rightX;
  BLUETOOTH_CONTROL_DATA_lastData.rightY = rightY;
  BLUETOOTH_CONTROL_DATA_lastData.button = button;

  return;
}
