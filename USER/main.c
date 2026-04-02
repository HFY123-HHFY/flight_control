#include "Bsp.h"

/*
407_飞控
*/

int main(void)                                                                      
{
	Bsp_Init();

	while(1)
	{
		// printf("Timer_Bsp_t:%d\r\n",Timer_Bsp_t);
		// usart_printf(USART3,"Pitch:%.1f, Roll:%.1f, Yaw:%.1f,QMC:%.1f,alt:%.1f\r\n",Pitch,Roll,Yaw,Angle_XY,alt);
/*
MPU6050:eda image copy occupy
*/
		mpu_angle(); // 读取MPU6050数据
		PID_Pitch_Roll_Combined(Pitch, Roll); // Pitch 和 Roll 合并双环控制函数
		// Set_PID(&pid_roll, 1.5f, 0.00f, 0.0f); // 设置Roll外环PID参数
		// Set_PID(&pid_rate_roll, 1.5f, 0.00f, 0.0f); // 设置Roll内环PID参数
		// Set_PID(&pid_pitch, 1.5f, 0.00f, 0.0f); // 设置Pitch外环PID参数
		// Set_PID(&pid_rate_pitch, 1.5f, 0.00f, 0.0f); // 设置Pitch内环PID参数

		// usart_printf(USART3, "pitch_output: %.1f,    roll.output: %.1f\r\n", pid_pitch.output, pid_roll.output);
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
无线串口调试:
*/	
		// USART3_Data();

		printf("Timer_Bsp_t:%d\r\n",Timer_Bsp_t);

/*
CAN总线调试:
*/
		// My_Can_TX_Data();
		// My_Can_RX_Data();
/*
OLED:
*/ 
		// OLED_Printf(0,0,OLED_8X16,"%d",Timer_Bsp_t);
		// OLED_Update();

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
	}
}
