#include "led.h" 

// LED初始化
void LED_Init(void)
{
    RCC->AHB1ENR |= 1<<4; // 使能GPIOE时钟
    GPIO_Set_Reg(GPIOE, GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4, REG_GPIO_MODE_OUTPUT, REG_GPIO_OTYPE_PP, REG_GPIO_SPEED_FAST, REG_GPIO_PUPD_UP);
    LED1 = 0; // 绿灯灭
    LED2 = 0; // 红灯灭
    LED3 = 0; // 蓝灯灭
}
