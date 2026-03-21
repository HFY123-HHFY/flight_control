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
	

	// QMC_Init();
	// QMC_EXTI_Init();
	
  // ControlTask_FusionInit(); // 卡尔曼滤波器初始化

  // BMP280Init();

  MPU_Init(); // 初始化MPU6050
  mpu_dmp_init(); // 初始化MPU6050 DMP
  MPU6050_EXTI_Init(); // 初始化MPU6050外部中断

  PWM_Init(PWM_TEST_ARR, PWM_TEST_PSC); // PWM初始化，20kHz（有刷电机）
  // PWM_Init(2000,840); // 频率50Hz，无刷电机
  PID_Contorl_Init(); // PID控制初始化
}
