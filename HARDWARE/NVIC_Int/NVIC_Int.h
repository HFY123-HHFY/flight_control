#ifndef __NVIC_INT_H
#define __NVIC_INT_H

#include "sys.h"
#include "LED.h"
#include "mpu6050.h"
#include "inv_mpu.h" // MPU6050 提供 MPU6050 的基本功能接口，例如初始化、数据读取、寄存器配置等
#include "inv_mpu_dmp_motion_driver.h" // MPU6050 DMP驱动，启用 MPU6050 的 DMP 功能，处理复杂的运动数据，例如姿态解算、陀螺仪和加速度计数据融合。

#define MPU6050_INT PEin(7) // MPU6050外部中断引脚
#define QMC_INT PEin(0) // QMC外部中断引脚

void MPU6050_EXTI_Init(void); // MPU6050外部中断初始化
void QMC_EXTI_Init(void); // QMC5883外部中断初始化

extern float Pitch,Roll,Yaw;	        // Pitch：俯仰角，Roll：横滚角，Yaw：偏航角
extern short gyrox,gyroy,gyroz;        // 角速度
extern short aacx,aacy,aacz;           // 加速度  short短整型
extern float alt; // 高度

extern volatile uint8_t mpu_flag; // MPU6050数据更新标志

void mpu_angle(void); // 读取MPU6050角度数据

#endif
