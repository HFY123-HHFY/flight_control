#ifndef __LC307_H
#define __LC307_H

#include "stm32f4xx.h" 

extern uint8_t g_lost_pos_dev;

uint8_t Opf_LC307_Init(void);
void Opf_LC307_Start(void);
void LC307_USART1_IdleHandler(void);
void LC307_USART1_RxByteHandler(uint8_t data);

__weak void getOpticalFlowResult_Callback(float *buf);



#endif
