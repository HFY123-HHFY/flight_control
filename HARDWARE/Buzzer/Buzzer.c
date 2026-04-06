#include "Buzzer.h"

//buzzer B1-TIM3-CH4

void Buzzer_Init(void)
{
    RCC->APB1ENR |= 1<<1; // 使能TIM3时钟
    RCC->AHB1ENR |= 1<<0; // 使能GPIOB时钟
    
    GPIO_Set_Reg(GPIOB, GPIO_Pin_1, REG_GPIO_MODE_AF, REG_GPIO_OTYPE_PP, REG_GPIO_SPEED_FAST, REG_GPIO_PUPD_UP);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource1, GPIO_AF_TIM3); // 将PB1配置为TIM3的复用功能
    
    TIM3->PSC = 84 - 1; // 定时器预分频，计数频率为84MHz/84=1MHz
    TIM3->ARR = (1000000U / 2700U) - 1; // 默认频率2700Hz
    TIM3->CCR4 = 0; // 初始占空比为0%
    TIM3->CCMR2 |= (6 << 12); // 配置CH4为PWM模式2
    TIM3->CCER |= (1 << 12); // 启用CH4输出
    TIM3->CR1 |= (1 << 0); // 启动定时器

    Buzzer_Control(500U, 2700U);
    delay_ms(300);
    Buzzer_Control(0U, 2700U);
}

// duty: 占空比千分比，范围0~1000；freq: 频率范围2500~2900Hz
void Buzzer_Control(uint16_t duty, uint16_t freq)
{
    uint32_t period_cnt;
    uint32_t ccr;

    if (freq < 2500U) freq = 2500U;
    if (freq > 2900U) freq = 2900U;
    if (duty > 1000) duty = 1000; // 占空比输入按0~1000的千分比

    period_cnt = 1000000U / freq;
    if (period_cnt < 1U) period_cnt = 1U;

    TIM3->ARR = period_cnt - 1U;

    ccr = (period_cnt * duty) / 1000U;
    if (ccr > TIM3->ARR) ccr = TIM3->ARR;
    TIM3->CCR4 = ccr;
}
