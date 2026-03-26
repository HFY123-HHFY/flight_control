#ifndef __ADC_H
#define __ADC_H	

#include "delay.h"	
#include "sys.h" 
#include "OLED.h"

void Adc_Init(void); 				//ADC通道初始化
u16  Get_Adc(u8 ch); 				//获得某个通道值 
u16 Get_Adc_Average(u8 ch,u8 times);//得到某个通道给定次数采样的平均值
void ADC_Get(void);

extern uint16_t ADValue[];			//定义AD值变量
extern float Voltage[];				//定义电压变量
extern uint32_t ADC_TimeoutCount;	//ADC等待超时计数
extern uint32_t ADC_ResErrorCount;	//ADC分辨率异常计数

#endif 
