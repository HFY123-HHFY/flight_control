#include "Pwm.h"

//设置定时器1的4个通道为PWM输出模式
void TIM1_PWM_Init(u32 arr, u32 psc)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_OCInitTypeDef  TIM_OCInitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE); 

    GPIO_PinAFConfig(GPIOE, GPIO_PinSource9, GPIO_AF_TIM1); 
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource11, GPIO_AF_TIM1);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource13, GPIO_AF_TIM1); 
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource14, GPIO_AF_TIM1);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF; 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; 
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; 
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_11 | GPIO_Pin_13 | GPIO_Pin_14; 
    GPIO_Init(GPIOE, &GPIO_InitStructure); 

    TIM_TimeBaseStructure.TIM_Period = arr - 1; 
    TIM_TimeBaseStructure.TIM_Prescaler = psc - 1; 
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; 
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

    TIM_OCStructInit(&TIM_OCInitStructure); // 初始化TIM_OCInitStructure结构体为默认值，避免未设置参数导致寄存器状态不确定
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 0;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

    // TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Disable;
    // TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;
    // TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;
    // TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Reset;

    TIM_OC1Init(TIM1, &TIM_OCInitStructure);
    TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);

    TIM_OC2Init(TIM1, &TIM_OCInitStructure);
    TIM_OC2PreloadConfig(TIM1, TIM_OCPreload_Enable);

    TIM_OC3Init(TIM1, &TIM_OCInitStructure);
    TIM_OC3PreloadConfig(TIM1, TIM_OCPreload_Enable);

    TIM_OC4Init(TIM1, &TIM_OCInitStructure);
    TIM_OC4PreloadConfig(TIM1, TIM_OCPreload_Enable);

    TIM_ARRPreloadConfig(TIM1, ENABLE);

    // 高级定时器还需要使能主输出
    TIM_CtrlPWMOutputs(TIM1, ENABLE);

    // 显式打开4个通道输出，避免寄存器状态受其它初始化影响
    // TIM_CCxCmd(TIM1, TIM_Channel_1, TIM_CCx_Enable);
    // TIM_CCxCmd(TIM1, TIM_Channel_2, TIM_CCx_Enable);
    // TIM_CCxCmd(TIM1, TIM_Channel_3, TIM_CCx_Enable);
    // TIM_CCxCmd(TIM1, TIM_Channel_4, TIM_CCx_Enable);

    TIM_Cmd(TIM1, ENABLE);
}

// 设置CCR- 占空比Duty = CCR / (ARR + 1)
void MOS1_Control(uint16_t duty)
{
    TIM1->CCR1 = duty;
}

void MOS2_Control(uint16_t duty)
{
    TIM1->CCR2 = duty;
}

void MOS3_Control(uint16_t duty)
{
    TIM1->CCR3 = duty;
}

void MOS4_Control(uint16_t duty)
{
    TIM1->CCR4 = duty;
}
