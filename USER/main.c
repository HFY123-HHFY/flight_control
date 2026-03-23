#include "Bsp.h"

	float P_kp = 0, P_ki = 0, P_kd = 0;
	float R_kp = 0, R_ki = 0, R_kd = 0;

/*
407_飞控
*/

int main(void)                                                                      
{
	Bsp_Init();

	while(1)
	{
	// printf("Timer_Bsp_t:%d\r\n",Timer_Bsp_t);
		
/*
MPU6050:
*/
	mpu_angle(); // 读取MPU6050角度数据
	PID_Pitch_Roll_Combined(Pitch, Roll); // Pitch 和 Roll 合并双环控制函数
	
	// Set_PID(&pid_pitch, 5.0f, 0.00f, 1.0f); // 设置pitch环的PID参数
	// Set_PID(&pid_roll, 5.0f, 0.00f, 1.0f); // 设置roll环的PID参数
		
/*
QMC5883L:
*/
	// Angle_XY = QMC_Data();
	// printf("%.2f\r\n",Angle_XY);
	
/*
BMP280:
*/
	// alt = BMP_Data();
	// printf("海拔: %.2f\r\n", alt);

/*
NRF24L01:
*/
	NRF24L01_Data();

/*
OLED:
*/
	// OLED_Clear();
	// OLED_Printf(0,0,OLED_8X16,"%d",Timer_Bsp_t);
	// OLED_Printf(64,0,OLED_8X16,"RX:%d",USART_3_RX);

	// OLED_Update();

/*
无线串口调试:
*/	
	if (uart3_flag == 1)
	{
		uart3_flag = 0;

		switch (USART_3_RX)
		{
		case 1:
			P_kp += 10.0f; // 增加KP
			break;
		
		case 2:
			P_kp -= 10.0f; // 减少KP
			break;

		case 3:
			R_kp += 10.0f; // 增加KP
			break;
		
		case 4:
			R_kp -= 10.0f; // 减少KP
			break;

		default:
			break;
		}
		// 更新Pitch环
        Set_PID(&pid_pitch, P_kp, 0, 0); 
        // 更新Roll环
        Set_PID(&pid_roll, R_kp, 0, 0);  
	}

//	usart_printf(USART3, "T:%d, Pitch_KP: %.1f, Roll_KP: %.1f\r\n", Timer_Bsp_t, P_kp, R_kp); // 打印当前PID参数

/*
电机调试:
MOS1-白-5V
MOS2-白-0V
MOS3-白-5V
MOS4-白-0V
*/
	// Motor_Test();
	//printf("speed_temp:%d\r\n",speed_temp);

/*
ADC电流环：
*/

	// ADC_Get();
	// Get_Adc(ADC_Channel_0); // 获取通道0的ADC值
	// Get_Adc(ADC_Channel_1); // 获取通道1的ADC值
	// Get_Adc(ADC_Channel_2); // 获取通道2的ADC值
	// Get_Adc(ADC_Channel_3); // 获取通道3的ADC值

	// OLED_Clear();

	// OLED_Printf(0,0,OLED_8X16,"Timer_Bsp_t:%d",Timer_Bsp_t);
	// OLED_Printf(84,0,OLED_8X16,"%.1f",Voltage[0]); // ADC滤波平均值
	// OLED_Printf(84,16,OLED_8X16,"%.1f",Voltage[1]);
	// OLED_Printf(84,32,OLED_8X16,"%.1f",Voltage[2]);
	// OLED_Printf(84,48,OLED_8X16,"%.1f",Voltage[3]);	
	// OLED_Update();
	}
}
