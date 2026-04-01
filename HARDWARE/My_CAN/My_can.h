#ifndef __MY_CAN_H
#define __MY_CAN_H

#include "stm32f4xx.h" 
#include "usart_3.h"

void My_CAN_Init(void);
void My_CAN_Transmit(uint32_t ID, uint8_t Length, uint8_t *Data);
uint8_t My_CAN_ReceiveFlag(void);
void My_CAN_Receive(uint32_t *ID, uint8_t *Length, uint8_t *Data);

void My_Can_TX_Data(void);
void My_Can_RX_Data(void);

#endif
