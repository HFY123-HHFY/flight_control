#include "led.h" 

void LED_Init(void)
{ 
    GPIO_InitTypeDef  GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    GPIO_ResetBits(GPIOE,GPIO_Pin_2);//低
    GPIO_ResetBits(GPIOE,GPIO_Pin_3);//低
    GPIO_ResetBits(GPIOE,GPIO_Pin_4);//低
}

void LED_on(int led)
{
    switch(led)
    {
        case 1: LED1 = 1; break;
        case 2: LED2 = 1; break;
        case 3: LED3 = 1; break;
        case 4: LED1 = 1, LED2 = 1, LED3 = 1; break;
        default: break;
    }
}

void LED_off(int led)
{
    switch(led)
    {
        case 1: LED1 = 0; break;
        case 2: LED2 = 0; break;
        case 3: LED3 = 0; break;
        case 4: LED1 = 0, LED2 = 0, LED3 = 0; break;
        default: break;
    }
}
