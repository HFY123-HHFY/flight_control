#include "key.h"

uint8_t Key_Num;
uint8_t Key = 0;

void Key_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA , ENABLE);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN; // 普通输入模式
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; // 100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; // 上拉
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

uint8_t Key_GetNum(void)
{
	uint8_t Temp;
	if (Key_Num)
	{
		Temp = Key_Num;
		Key_Num = 0;
		return Temp;
	}
	return 0;
}

uint8_t Key_GetState(void)
{
	if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == 0)
	{
		return 1; // KEY
	}
	return 0;
}

void Key_Tick(void)
{
	static uint8_t Count; // 按键计数器
	static uint8_t CurrState, PrevState; // 当前状态和前一状态
	
	Count ++;
	if (Count >= 20)
	{
		Count = 0;
		
		PrevState = CurrState;
		CurrState = Key_GetState();
		
		if (CurrState == 0 && PrevState != 0)
		{
			Key_Num = PrevState;
		}
	}
}

void key_Get(void)
{
	uint8_t KeyNum = 0;
	
	KeyNum = Key_GetNum();
	if(KeyNum)
	{
		Key = KeyNum;
		KeyNum = 0;
	}

	if (Key == 1)
	{
		speed_temp += 50;
		Key = 0;
	}
}
