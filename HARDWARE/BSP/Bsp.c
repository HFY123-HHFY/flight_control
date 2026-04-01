#include "Bsp.h"

uint32_t Timer_Bsp_t = 0; // 程序运行的时间

void Bsp_Init(void)
{
  delay_Init(168);
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
  TIM3_4_Init(1000,84); // 定时器3和4初始化，1ms中断一次
  
  LED_Init();
  // Key_Init();
	Adc_Init();
	
  usart_1_Init(9600); // 板载调试
  usart_3_Init(115200); // 无线串口调试

  MyI2C_Init();
  delay_us(10);
	
	OLED_Init();

  NRF24L01_Init(); // 初始化NRF24L01
	
	QMC_Init();
	// QMC_EXTI_Init();
	
  // ControlTask_FusionInit(); // 卡尔曼滤波器初始化

  BMP280Init();

  MPU_Init(); // 初始化MPU6050
  mpu_dmp_init(); // 初始化MPU6050 DMP
  MPU6050_EXTI_Init(); // 初始化MPU6050外部中断

  TIM1_PWM_Init_Reg_WithGpioWrap(PWM_TEST_ARR, PWM_TEST_PSC);

  PID_Contorl_Init(); // PID控制初始化
}
