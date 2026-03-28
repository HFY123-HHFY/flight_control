#include "usart_3.h"

/*
 无线串口调试
*/

uint32_t USART_3_RX = 0;
uint8_t uart3_flag = 0; // 串口3接收标志位

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
		uart3_flag = 1; // 设置串口3接收标志位
	}
	USART_ClearITPendingBit(USART3, USART_IT_RXNE);
}

float R_kp = 1.8, R_ki = 0.0, R_kd = 1.90; // Roll正侧PID参数
float R_kp_n = 0.4, R_ki_n = 0.0, R_kd_n = 0.3; // Roll负侧PID参数

void USART3_Data(void)
{
	if (uart3_flag == 1)
	{
		uart3_flag = 0;
		switch (USART_3_RX)
		{
		// ROLL < 0
		case 'a':	R_kp += 0.1f;	break;
		
		case 'b':	R_kp -= 0.1f;
			if (R_kp < 0.0f)
			{
				R_kp = 0.0f;
			}
			break;

		case 'c':	R_ki += 0.002f;	break;
			
		case 'd':
			R_ki -= 0.002f;
			if (R_ki < 0.0f)
			{
				R_ki = 0.0f;
			}
			break;

		case 'e':	R_kd += 0.1f;	break;
			
		case 'f':	R_kd -= 0.1f;
			if (R_kd < 0.0f)
			{
				R_kd = 0.0f;
			}
			break;

// ROLL >= 0
		case 'g':	R_kp_n += 0.1f;	break;

		case 'h':	R_kp_n -= 0.1f;
			if (R_kp_n < 0.0f)
			{
				R_kp_n = 0.0f;
			}
			break;

		case 'i':	R_ki_n += 0.002f;	break;

		case 'j':	R_ki_n -= 0.002f;
			if (R_ki_n < 0.0f)
			{
				R_ki_n = 0.0f;
			}
			break;

		case 'k':	R_kd_n += 0.1f;	break;
			
		case 'l':	R_kd_n -= 0.1f;
			if (R_kd_n < 0.0f)
			{
				R_kd_n = 0.0f;
			}
			break;

		default:
			break;
		}
		// 更新Roll环
		Set_Roll_BiPID(R_kp, R_ki, R_kd, R_kp_n, R_ki_n, R_kd_n);
		usart_printf(USART3,"T:%d, R:%.1f, P+(%.1f,%.2f,%.1f), P-(%.1f,%.2f,%.1f), out:%.1f\r\n",
		Timer_Bsp_t, Roll, R_kp, R_ki, R_kd, R_kp_n, R_ki_n, R_kd_n, pid_roll.output);
	}
}
