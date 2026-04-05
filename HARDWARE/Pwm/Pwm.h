#ifndef __pwm_H
#define __pwm_H

#include "sys.h"

/*
DShot300 参数:
TIM1 时钟为 168MHz, 目标 PWM 频率为 300kHz。
本工程的 TIM1_PWM_Init() 内部采用 arr-1 / psc-1 的写法,
因此这里传入的参数应为: ARR=560, PSC=1。
实际寄存器值分别为 ARR=559, PSC=0。
*/
#define PWM_TEST_ARR                 560U
#define PWM_TEST_PSC                 1U

/* DShot 油门定义: 0 停转, 1~47 为命令, 48~2047 为正常油门 */
#define DSHOT_THROTTLE_MIN           48U
#define DSHOT_THROTTLE_MAX           2047U
#define PWM_DUTY_MAX                 ((uint16_t)DSHOT_THROTTLE_MAX)

void TIM1_PWM_Init(u32 arr, u32 psc); // 寄存器+GPIO封装方式: TIM1四通道PWM初始化

void MOS1_Control(uint16_t duty);
void MOS2_Control(uint16_t duty);
void MOS3_Control(uint16_t duty);
void MOS4_Control(uint16_t duty);

/* 一次性发送4路电机的DShot油门数据 */
void TIM1_DShot_Write(uint16_t m1, uint16_t m2, uint16_t m3, uint16_t m4);

#endif
