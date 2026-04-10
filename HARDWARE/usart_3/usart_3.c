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
		USART_ClearITPendingBit(USART3, USART_IT_RXNE); // 清除中断标志位
	}

	if(USART_GetITStatus(USART3, USART_IT_TXE) == SET)
	{
		// 发送寄存器空: 交给通用异步发送处理函数继续从队列出队发送
		usart_tx_irq_handler(USART3);
	}
}

void USART3_Data(void)
{
	static float kp = 4.0f, ki = 0.0f, kd = 0.0f;// 外环PID参数
	static float rate_kp = 0.0f, rate_ki = 0.00f, rate_kd = 0.00f; // 内环PID参数

	if (uart3_flag == 1)
	{
		uart3_flag = 0;
		switch (USART_3_RX)
		{
	/*
	内环PID参数调整：
	*/
			case 'a':	
				rate_kp += 0.1f;	
			break;
			
			case 'b':	
			rate_kp -= 0.1f;
			if (rate_kp < 0.0f)
			{
				rate_kp = 0.0f;
			}
			break;

			case 'c':	
				rate_ki += 0.01f;
			break;
				
			case 'd':
				rate_ki -= 0.01f;
				if (rate_ki < 0.0f)
				{
					rate_ki = 0.0f;
				}
			break;

			case 'e':	
				rate_kd += 0.01f;	
			break;
				
			case 'f':	
				rate_kd -= 0.01f;
				if (rate_kd < 0.0f)
				{
					rate_kd = 0.0f;
				}
			break;
			case 'i':	
				rate_kd += 0.1f;	
			break;
				
			case 'j':	
				rate_kd -= 0.1f;
				if (rate_kd < 0.0f)
				{
					rate_kd = 0.0f;
				}	
	/*
	外环PID参数调整：
	*/
			case 'g':	
				kp += 0.2f;	
			break;
			case 'h':	
				kp -= 0.2f;
				if (kp < 0.0f)
				{
					kp = 0.0f;
				}
			break;
			
			default:
				break;
		}
		// 更新PID参数
		Set_PID(&pid_pitch, kp, ki, kd); // 设置Pitch外环PID参数
		Set_PID(&pid_rate_pitch, rate_kp, rate_ki, rate_kd); // 设置Pitch内环PID参数
		Set_PID(&pid_roll, kp, ki, kd); // 设置Roll外环PID参数
		Set_PID(&pid_rate_roll, rate_kp, rate_ki, rate_kd); // 设置Roll内环PID参数
	}

	if (print_task_flag)
	{
		print_task_flag = 0;
		printf("Roll: %.1f, kp: %.1f, rate_kp %.1f, rate_ki: %.2f, rate_kd:%.2f, out:%.1f\n",Roll, pid_roll.kp, pid_rate_roll.kp, pid_rate_roll.ki, pid_rate_roll.kd, pid_rate_roll.output);
		// printf("Pitch: %.1f, kp: %.1f, rate_kp: %.1f, rate_kp:: %.2f, rate_kd:%.2f, out:%.1f\n",Pitch, pid_pitch.kp, pid_rate_pitch.kp, pid_rate_pitch.ki, pid_rate_pitch.kd, pid_pitch.output);
	}
}
