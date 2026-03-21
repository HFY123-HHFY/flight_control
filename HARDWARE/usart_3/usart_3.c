#include "usart_3.h"

/*
 无线串口调试
*/

uint32_t USART_3_RX = 0;

//串口3初始化
void usart_3_Init(u32 bound_3)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD,ENABLE); //使能GPIOD时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);//使能USART3时钟

	GPIO_PinAFConfig(GPIOD,GPIO_PinSource8,GPIO_AF_USART3);
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource9,GPIO_AF_USART3);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9; //PD8,PD9
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; //速度100MHz
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF; //复用功能
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = bound_3;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART3, &USART_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStructure);

	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//开启中断
	USART_Cmd(USART3, ENABLE);                  //使能串口 
}

// 串口3中断服务函数
void USART3_IRQHandler(void) 
{
	if(USART_GetITStatus(USART3, USART_IT_RXNE) == SET) 
	{
		USART_3_RX = USART_ReceiveData(USART3); // 读取接收到的数据
	}
	USART_ClearITPendingBit(USART3, USART_IT_RXNE);
}
