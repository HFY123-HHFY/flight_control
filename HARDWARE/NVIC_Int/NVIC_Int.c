#include "NVIC_Int.h"

/*
MPU6050外部中断,函数变量定义
*/
float Pitch, Roll, Yaw;	        // Pitch：俯仰角，Roll：横滚角，Yaw：偏航角
short gyrox, gyroy, gyroz;        // 角速度,x轴、y轴、z轴
short aacx, aacy, aacz;           // 加速度 ,x轴、y轴、z轴
//short短整型，16位有符号整数，范围-32768~32767，单位：m/s^2, %hd

uint8_t volatile mpu_flag = 0; // MPU6050数据更新标志

 // MPU6050外部中断初始化
void MPU6050_EXTI_Init(void)
{
	NVIC_InitTypeDef   NVIC_InitStructure;
	EXTI_InitTypeDef   EXTI_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);//使能SYSCFG时钟

	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource7);

	/* 配置EXTI_Line7 */
	EXTI_InitStructure.EXTI_Line = EXTI_Line7;//LINE7
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//中断事件
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; //上升沿触发 
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;//使能LINE7
	EXTI_Init(&EXTI_InitStructure);//配置

	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;//外部中断7
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;//抢占优先级0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;//子优先级2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//使能外部中断通道
	NVIC_Init(&NVIC_InitStructure);//配置
}

 // QMC5883外部中断初始化
void QMC_EXTI_Init(void)
{
	NVIC_InitTypeDef   NVIC_InitStructure;
	EXTI_InitTypeDef   EXTI_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);//使能SYSCFG时钟

	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource0);

	/* 配置EXTI_Line0 */
	EXTI_InitStructure.EXTI_Line = EXTI_Line0;//LINE0
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//中断事件
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; //上升沿触发 
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;//使能LINE0
	EXTI_Init(&EXTI_InitStructure);//配置

	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;//外部中断0
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;//抢占优先级0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//使能外部中断通道
	NVIC_Init(&NVIC_InitStructure);//配置
}

void mpu_angle(void)
{
	if (mpu_flag == 1)
	{
		mpu_flag = 0; // 清除MPU6050数据更新标志
		mpu_dmp_get_data(&Pitch,&Roll,&Yaw);	    // 读取角度
		// MPU_Get_Gyroscope(&gyrox,&gyroy,&gyroz);  // 读取角速度
     	// MPU_Get_Accelerometer(&aacx,&aacy,&aacz); // 读取加速度
	}	
}
