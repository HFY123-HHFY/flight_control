#ifndef __USART_3_H
#define __USART_3_H

#include "stm32f4xx.h" 
#include "usart.h"
#include "Motor.h"

void usart_3_Init(u32 bound_3);

extern uint32_t USART_3_RX;

#endif
