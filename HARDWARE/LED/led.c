#include "led.h" 

// 库函数方式: LED初始化
/*
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
*/

// 直接操作寄存器方式: LED初始化
void LED_Init(void)
{
    RCC->AHB1ENR |= 1<<4; // 使能GPIOE时钟
    GPIO_Set_Reg(GPIOE, GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4, REG_GPIO_MODE_OUTPUT, REG_GPIO_OTYPE_PP, REG_GPIO_SPEED_FAST, REG_GPIO_PUPD_UP);
    LED1 = 0; // 绿灯灭
    LED2 = 0; // 红灯灭
    LED3 = 0; // 蓝灯灭
}
