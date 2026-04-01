#ifndef __pwm_H
#define __pwm_H

#include "sys.h"

/*
测试PWM参数（50Hz）:
f = 84MHz / PSC / ARR = 168MHz / 16800 / 200 = 50Hz
*/
#define PWM_TEST_ARR                 200U
#define PWM_TEST_PSC                 16800U
#define PWM_DUTY_MAX                 ((uint16_t)PWM_TEST_ARR)

// void TIM1_PWM_Init(u32 arr, u32 psc); // 标准库方式: TIM1四通道PWM初始化
// void TIM1_PWM_Init_Reg(u32 arr, u32 psc); // 寄存器方式: TIM1四通道PWM初始化
void TIM1_PWM_Init(u32 arr, u32 psc); // 寄存器+GPIO封装方式: TIM1四通道PWM初始化

void MOS1_Control(uint16_t duty);
void MOS2_Control(uint16_t duty);
void MOS3_Control(uint16_t duty);
void MOS4_Control(uint16_t duty);

#endif
