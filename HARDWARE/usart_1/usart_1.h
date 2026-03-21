#ifndef __USART_1_H
#define __USART_1_H

#include "stm32f4xx.h" 
#include "usart.h"
#include "Motor.h"

void usart_1_Init(u32 bound_1);

extern uint32_t USART_1_RX;

#endif
