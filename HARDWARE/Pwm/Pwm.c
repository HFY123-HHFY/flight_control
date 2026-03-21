#include "Pwm.h"

/*
    MOS1 B6-T4-CH1
    MOS2-B7-T4-CH2
    MOS3-D14-T4-CH3
    MOS4-D15-T4-CH4
*/

//设置定时器4的2个通道为PWM输出模式
void TIM4_PWM_Init(u32 arr, u32 psc)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_OCInitTypeDef  TIM_OCInitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE); 

    GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_TIM4); 
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_TIM4);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF; 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; 
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; 
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7; 
    GPIO_Init(GPIOB, &GPIO_InitStructure); 

    TIM_TimeBaseStructure.TIM_Period = arr - 1; 
    TIM_TimeBaseStructure.TIM_Prescaler = psc - 1; 
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; 
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 0;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

    TIM_OC1Init(TIM4, &TIM_OCInitStructure);
    TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);

    TIM_OC2Init(TIM4, &TIM_OCInitStructure);
    TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Enable);

    TIM_ARRPreloadConfig(TIM4, ENABLE);
    TIM_Cmd(TIM4, ENABLE);
}

//设置定时器9的2个通道为PWM输出模式
void TIM9_PWM_Init(u32 arr, u32 psc)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_OCInitTypeDef  TIM_OCInitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM9, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE); 

    GPIO_PinAFConfig(GPIOE, GPIO_PinSource5, GPIO_AF_TIM9); 
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource6, GPIO_AF_TIM9);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF; 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; 
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; 
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6; 
    GPIO_Init(GPIOE, &GPIO_InitStructure); 

    TIM_TimeBaseStructure.TIM_Period = arr - 1; 
    TIM_TimeBaseStructure.TIM_Prescaler = psc - 1; 
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; 
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  
    TIM_TimeBaseInit(TIM9, &TIM_TimeBaseStructure);

    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 0;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

    TIM_OC1Init(TIM9, &TIM_OCInitStructure);
    TIM_OC1PreloadConfig(TIM9, TIM_OCPreload_Enable);

    TIM_OC2Init(TIM9, &TIM_OCInitStructure);
    TIM_OC2PreloadConfig(TIM9, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM9, ENABLE);
    TIM_Cmd(TIM9, ENABLE);
}
// 总PWM初始化
void PWM_Init(u32 arr, u32 psc)
{
    TIM4_PWM_Init(arr, psc);
    TIM9_PWM_Init(arr, psc);
}

// 设置CCR- 占空比Duty = CCR / (ARR + 1)
void MOS1_Control(uint16_t duty)
{
    TIM4->CCR1 = duty;
}

void MOS2_Control(uint16_t duty)
{
    TIM4->CCR2 = duty;
}

void MOS3_Control(uint16_t duty)
{
    TIM9->CCR1 = duty;
}

void MOS4_Control(uint16_t duty)
{
    TIM9->CCR2 = duty;
}
