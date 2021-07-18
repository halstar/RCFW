#ifndef __BLUETOOTH_CONTROL_H
#define __BLUETOOTH_CONTROL_H

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

#define PSB_SELECT      1
#define PSB_L3          2
#define PSB_R3          3
#define PSB_START       4
#define PSB_PAD_UP      5
#define PSB_PAD_RIGHT   6
#define PSB_PAD_DOWN    7
#define PSB_PAD_LEFT    8
#define PSB_L2          9
#define PSB_R2          10
#define PSB_L1          11
#define PSB_R1          12
#define PSB_GREEN       13
#define PSB_RED         14
#define PSB_BLUE        15
#define PSB_PINK        16

#define PSB_TRIANGLE    13
#define PSB_CIRCLE      14
#define PSB_CROSS       15
#define PSB_SQUARE      16

#define PSS_RX 5
#define PSS_RY 6
#define PSS_LX 7
#define PSS_LY 8


void BLUETOOTH_CONTROL_init(void);
void BLUETOOTH_CONTROL_receiveData(void);


#endif /* __BLUETOOTH_CONTROL_H */
