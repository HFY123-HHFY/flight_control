#ifndef __NVIC_INT_H
#define __NVIC_INT_H

#include "sys.h"
#include "LED.h"

#define MPU6050_INT PEin(7) // MPU6050外部中断引脚
#define QMC_INT PEin(0) // QMC外部中断引脚

void MPU6050_EXTI_Init(void); // MPU6050外部中断初始化
void QMC_EXTI_Init(void); // QMC5883外部中断初始化

extern float Pitch,Roll,Yaw;	        // Pitch：俯仰角，Roll：横滚角，Yaw：偏航角
extern short gyrox,gyroy,gyroz;        // 角速度
extern short aacx,aacy,aacz;           // 加速度  short短整型
extern float alt; // 高度

extern int mpu_flag; // MPU6050数据更新标志

#endif
