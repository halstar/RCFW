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

uint8_t BLUETOOTH_CONTROL_buffer[9]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
uint16_t BLUETOOTH_CONTROL_mask[]={
    PSB_SELECT,
    PSB_L3,
    PSB_R3 ,
    PSB_START,
    PSB_PAD_UP,
    PSB_PAD_RIGHT,
    PSB_PAD_DOWN,
    PSB_PAD_LEFT,
    PSB_L2,
    PSB_R2,
    PSB_L1,
    PSB_R1 ,
    PSB_GREEN_TRIANGLE,
    PSB_RED_CIRCLE,
    PSB_BLUE_CROSS,
    PSB_PINK_SQUARE
};

#define BLUETOOTH_CONTROL_RIGHT_X 5
#define BLUETOOTH_CONTROL_RIGHT_Y 6
#define BLUETOOTH_CONTROL_LEFT_X  7
#define BLUETOOTH_CONTROL_LEFT_Y  8

uint8_t PS2_LX, PS2_LY, PS2_RX, PS2_RY, PS2_KEY;

static void    BLUETOOTH_CONTROL_readData(void);
static void    BLUETOOTH_CONTROL_sendCommand(uint8_t command);
static uint8_t BLUETOOTH_CONTROL_getDataKey(void);
static void    BLUETOOTH_CONTROL_clearData(void);

void BLUETOOTH_CONTROL_sendCommand(uint8_t command)
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

void BLUETOOTH_CONTROL_readData(void)
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

uint8_t BLUETOOTH_CONTROL_getDataKey()
{
  uint8_t index;
  uint16_t handkey;

  BLUETOOTH_CONTROL_clearData();
  BLUETOOTH_CONTROL_readData();
  handkey=(BLUETOOTH_CONTROL_buffer[4]<<8)|BLUETOOTH_CONTROL_buffer[3];
  for(index=0;index<16;index++)
  {
    if((handkey&(1<<(BLUETOOTH_CONTROL_mask[index]-1)))==0)
    return index+1;
  }

  return 0;
}

void BLUETOOTH_CONTROL_clearData()
{
  uint8_t a;
  for(a=0;a<9;a++)
    BLUETOOTH_CONTROL_buffer[a]=0x00;

  return;
}

void BLUETOOTH_CONTROL_init(void)
{
  LOG_info("Initializing Bluetooth control");

  RCC->APB2ENR|=1<<2;     // PORTA
  RCC->APB2ENR|=1<<4;     // PORTC
  GPIOC->CRL&=0XFFFFF0FF;
  GPIOC->CRL|=0X00000800;

  RCC->APB2ENR|=1<<4;    // PORTC
  GPIOC->CRL&=0XFFFF0F0F;
  GPIOC->CRL|=0X00003030; // PC1 PC3

  GPIOA->CRL&=0XFFF0FFFF;
  GPIOA->CRL|=0X00030000; // PA4

  return;
}

void BLUETOOTH_CONTROL_receiveData(void)
{
  LOG_info("Receiving Bluetooth data");

  PS2_LX = BLUETOOTH_CONTROL_buffer[BLUETOOTH_CONTROL_LEFT_X];
  PS2_LY = BLUETOOTH_CONTROL_buffer[BLUETOOTH_CONTROL_LEFT_Y];
  PS2_RX = BLUETOOTH_CONTROL_buffer[BLUETOOTH_CONTROL_RIGHT_X];
  PS2_RY = BLUETOOTH_CONTROL_buffer[BLUETOOTH_CONTROL_RIGHT_Y];

  PS2_KEY = BLUETOOTH_CONTROL_getDataKey();

  return;
}
