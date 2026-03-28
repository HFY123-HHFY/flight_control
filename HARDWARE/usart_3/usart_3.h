#ifndef __USART_3_H
#define __USART_3_H

#include "stm32f4xx.h" 
#include "usart.h"
#include "Motor.h"
#include "Bsp.h"

void usart_3_Init(u32 bound_3);
void USART3_Data(void); // 串口3调试PID参数

extern uint32_t USART_3_RX;
extern uint8_t uart3_flag; // 串口3接收标志位

#endif
