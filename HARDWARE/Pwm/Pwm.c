#include "Pwm.h"

//库函数方式:设置定时器1的4个通道为PWM输出模式
/*
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
*/

// 寄存器方式: TIM1四通道PWM初始化
/*
void TIM1_PWM_Init_Reg(u32 arr, u32 psc)
{
    // 1) 打开时钟: TIM1在APB2, GPIOE在AHB1
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;

    // 2) PE9/11/13/14 复用为TIM1_CH1/2/3/4
    // MODER: 10=复用功能
    GPIOE->MODER &= ~((3U << (9U * 2U)) |
                      (3U << (11U * 2U)) |
                      (3U << (13U * 2U)) |
                      (3U << (14U * 2U)));
    GPIOE->MODER |=  ((2U << (9U * 2U)) |
                      (2U << (11U * 2U)) |
                      (2U << (13U * 2U)) |
                      (2U << (14U * 2U)));

    // 推挽输出, 高速, 上拉
    GPIOE->OTYPER &= ~((1U << 9U) | (1U << 11U) | (1U << 13U) | (1U << 14U));

    GPIOE->OSPEEDR &= ~((3U << (9U * 2U)) |
                        (3U << (11U * 2U)) |
                        (3U << (13U * 2U)) |
                        (3U << (14U * 2U)));
    GPIOE->OSPEEDR |=  ((3U << (9U * 2U)) |
                        (3U << (11U * 2U)) |
                        (3U << (13U * 2U)) |
                        (3U << (14U * 2U)));

    GPIOE->PUPDR &= ~((3U << (9U * 2U)) |
                      (3U << (11U * 2U)) |
                      (3U << (13U * 2U)) |
                      (3U << (14U * 2U)));
    GPIOE->PUPDR |=  ((1U << (9U * 2U)) |
                      (1U << (11U * 2U)) |
                      (1U << (13U * 2U)) |
                      (1U << (14U * 2U)));

    // AFR[1]中配置AF1(TIM1): pin9/11/13/14
    GPIOE->AFR[1] &= ~((0xFU << ((9U - 8U) * 4U)) |
                       (0xFU << ((11U - 8U) * 4U)) |
                       (0xFU << ((13U - 8U) * 4U)) |
                       (0xFU << ((14U - 8U) * 4U)));
    GPIOE->AFR[1] |=  ((0x1U << ((9U - 8U) * 4U)) |
                       (0x1U << ((11U - 8U) * 4U)) |
                       (0x1U << ((13U - 8U) * 4U)) |
                       (0x1U << ((14U - 8U) * 4U)));

    // 3) 定时器基准参数: f_pwm = tim_clk / ((PSC+1)*(ARR+1))
    TIM1->PSC = psc - 1U;
    TIM1->ARR = arr - 1U;

    // 4) PWM模式1 + 预装载使能
    // OCxM=110(PWM1), OCxPE=1
    TIM1->CCMR1 &= ~(TIM_CCMR1_OC1M | TIM_CCMR1_OC2M);
    TIM1->CCMR1 |=  (TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1PE |
                     TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2PE);

    TIM1->CCMR2 &= ~(TIM_CCMR2_OC3M | TIM_CCMR2_OC4M);
    TIM1->CCMR2 |=  (TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3PE |
                     TIM_CCMR2_OC4M_1 | TIM_CCMR2_OC4M_2 | TIM_CCMR2_OC4PE);

    // 5) 打开4路主输出通道
    TIM1->CCER |= (TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E | TIM_CCER_CC4E);

    // 初始占空比=0
    TIM1->CCR1 = 0U;
    TIM1->CCR2 = 0U;
    TIM1->CCR3 = 0U;
    TIM1->CCR4 = 0U;

    // 6) 高级定时器必须置位MOE，否则不会从引脚输出
    TIM1->BDTR |= TIM_BDTR_MOE;

    // 7) 使能ARR预装载，产生一次更新事件，把预装载值同步到影子寄存器
    TIM1->CR1 |= TIM_CR1_ARPE;
    TIM1->EGR |= TIM_EGR_UG;

    // 8) 启动计数器
    TIM1->CR1 |= TIM_CR1_CEN;
}
*/

// 寄存器+GPIO封装方式: TIM1四通道PWM初始化
void TIM1_PWM_Init(u32 arr, u32 psc)
{
    // 1) 开时钟
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;

    // 2) 通过通用GPIO封装配置PE9/11/13/14
    GPIO_Set_Reg(GPIOE,
                 (uint16_t)((1U << 9U) | (1U << 11U) | (1U << 13U) | (1U << 14U)),
                 REG_GPIO_MODE_AF,
                 REG_GPIO_OTYPE_PP,
                 REG_GPIO_SPEED_HIGH,
                 REG_GPIO_PUPD_UP);

    GPIO_AF_Set_Reg(GPIOE, 9U, 1U);
    GPIO_AF_Set_Reg(GPIOE, 11U, 1U);
    GPIO_AF_Set_Reg(GPIOE, 13U, 1U);
    GPIO_AF_Set_Reg(GPIOE, 14U, 1U);

    // 3) 定时器基准频率: f = tim_clk / ((PSC+1)*(ARR+1))
    TIM1->PSC = psc - 1U;
    TIM1->ARR = arr - 1U;

    // 4) PWM1 + CCR预装载(4通道)
    TIM1->CCMR1 &= ~(TIM_CCMR1_OC1M | TIM_CCMR1_OC2M);
    TIM1->CCMR1 |= (TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1PE |
                    TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2PE);

    TIM1->CCMR2 &= ~(TIM_CCMR2_OC3M | TIM_CCMR2_OC4M);
    TIM1->CCMR2 |= (TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3PE |
                    TIM_CCMR2_OC4M_1 | TIM_CCMR2_OC4M_2 | TIM_CCMR2_OC4PE);

    // 5) 打开4个输出通道
    TIM1->CCER |= (TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E | TIM_CCER_CC4E);

    // 6) 初始占空比为0
    TIM1->CCR1 = 0U;
    TIM1->CCR2 = 0U;
    TIM1->CCR3 = 0U;
    TIM1->CCR4 = 0U;

    // 7) 高级定时器需要MOE
    TIM1->BDTR |= TIM_BDTR_MOE;

    // 8) 预装载生效并启动
    TIM1->CR1 |= TIM_CR1_ARPE;
    TIM1->EGR |= TIM_EGR_UG;
    TIM1->CR1 |= TIM_CR1_CEN;
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
