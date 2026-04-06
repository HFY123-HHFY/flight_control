#ifndef __BSP_H
#define __BSP_H

#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "Timer.h"

#include "NVIC_IRQ.h"
#include "usart_1.h"  
#include "usart_2.h"  
#include "usart_3.h"
#include "adc.h"

#include "led.h"
#include "Buzzer.h"
#include "key.h"
#include "OLED.h"

#include "Control.h"
#include "Control_task.h"
#include "Pwm.h"
#include "Motor.h"

#include "mpu6050.h"
#include "inv_mpu.h" // MPU6050 提供 MPU6050 的基本功能接口，例如初始化、数据读取、寄存器配置等
#include "inv_mpu_dmp_motion_driver.h" // MPU6050 DMP驱动，启用 MPU6050 的 DMP 功能，处理复杂的运动数据，例如姿态解算、陀螺仪和加速度计数据融合。
#include "NVIC_Int.h"// 外部中断处理

#include "bmp280.h"
#include "QMC5883P.h"
#include "LC307.h"

#include "NRF24L01.h"
#include "My_can.h"

void Bsp_Init(void);

extern uint32_t Timer_Bsp_t; // 程序运行的时间

#endif
