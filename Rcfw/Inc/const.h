#ifndef __CONST_H
#define __CONST_H

#define CST_FRONT_RIGHT_MOTOR_NAME "FRONT RIGHT"
#define CST_FRONT_LEFT_MOTOR_NAME  "FRONT LEFT "
#define CST_REAR_LEFT_MOTOR_NAME   "REAR  LEFT "
#define CST_REAR_RIGHT_MOTOR_NAME  "REAR  RIGHT"

#define CST_CONSOLE_TX_MAX_STRING_LENGTH 128
#define CST_MASTER_TX_MAX_STRING_LENGTH   CST_CONSOLE_TX_MAX_STRING_LENGTH
#define CST_MASTER_RX_MAX_STRING_LENGTH   16
#define CST_STRING_FIFO_MAX_STRING_LENGTH CST_MASTER_RX_MAX_STRING_LENGTH

#define CST_CIRCULAR_BUFFER_MAX_ELEMENTS 10
#define CST_STRING_FIFO_MAX_ELEMENTS     10

#endif /* __CONST_H */
