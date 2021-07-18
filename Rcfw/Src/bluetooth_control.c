#include "bluetooth_control.h"

#include "stm32f1xx_hal.h"
#include "utils.h"

uint8_t BLUETOOTH_CONTROL_rxBuffer[12] = {0};

uint16_t Handkey;
uint8_t Comd[2]={0x01,0x42};
uint8_t Data[9]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
uint16_t MASK[]={
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
    PSB_GREEN,
    PSB_RED,
    PSB_BLUE,
    PSB_PINK
};
uint8_t PS2_LX, PS2_LY, PS2_RX, PS2_RY, PS2_KEY;


static void PS2_ReadData(void);
static void PS2_Cmd(uint8_t CMD);
static uint8_t PS2_DataKey(void);
static void PS2_ClearData(void);

void PS2_Cmd(uint8_t CMD)
{
  volatile uint16_t ref=0x01;
  Data[1] = 0;
  for(ref=0x01;ref<0x0100;ref<<=1)
  {
    if(ref&CMD)
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
      Data[1] = ref|Data[1];
  }
  UTILS_delayUs(16);
}

void PS2_ReadData(void)
{
  volatile uint8_t byte=0;
  volatile uint16_t ref=0x01;
  CS_L;
  PS2_Cmd(Comd[0]);
  PS2_Cmd(Comd[1]);
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
          Data[byte] = ref|Data[byte];
    }
        UTILS_delayUs(16);
  }
  CS_H;
}

uint8_t PS2_DataKey()
{
  uint8_t index;

  PS2_ClearData();
  PS2_ReadData();

  Handkey=(Data[4]<<8)|Data[3];
  for(index=0;index<16;index++)
  {
    if((Handkey&(1<<(MASK[index]-1)))==0)
    return index+1;
  }

  return 0;
}

void PS2_ClearData()
{
  uint8_t a;
  for(a=0;a<9;a++)
    Data[a]=0x00;
}

void BLUETOOTH_CONTROL_init(void)
{
  RCC->APB2ENR|=1<<2;     // PORTA
  RCC->APB2ENR|=1<<4;     // PORTC
  GPIOC->CRL&=0XFFFFF0FF;
  GPIOC->CRL|=0X00000800;

  RCC->APB2ENR|=1<<4;    // PORTC
  GPIOC->CRL&=0XFFFF0F0F;
  GPIOC->CRL|=0X00003030; // PC1 PC3

  GPIOA->CRL&=0XFFF0FFFF;
  GPIOA->CRL|=0X00030000; // PA4
}

void BLUETOOTH_CONTROL_receiveData(void)
{
  PS2_LX = Data[PSS_LX];
  PS2_LY = Data[PSS_LY];
  PS2_RX = Data[PSS_RX];
  PS2_RY = Data[PSS_RY];

  PS2_KEY = PS2_DataKey();
}
