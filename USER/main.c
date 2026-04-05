#include "Bsp.h"

/*
407_飞控
*/

int main(void)
{
	Bsp_Init();

	while (1)
	{
		// printf("Timer_Bsp_t:%d\r\n",Timer_Bsp_t);
		// usart_printf(USART3,"Pitch:%.1f, Roll:%.1f, Yaw:%.1f,QMC:%.1f,alt:%.1f\r\n",Pitch,Roll,Yaw,Angle_XY,alt);
/*
MPU6050:
*/
		mpu_angle(); // 读取MPU6050数据
		PID_Pitch_Roll_Combined(Pitch, Roll); // Pitch 和 Roll 合并双环控制函数
		Set_PID(&pid_roll, 12.0f, 0.00f, 0.0f); // 设置Roll外环PID参数
		Set_PID(&pid_rate_roll, 6.0f, 0.00f, 0.0f); // 设置Roll内环PID参数
		Set_PID(&pid_pitch, 12.0f, 0.00f, 0.0f); // 设置Pitch外环PID参数
		Set_PID(&pid_rate_pitch, 6.0f, 0.00f, 0.0f); // 设置Pitch内环PID参数
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
LC307光流:
*/
		// LC307_Speed_Control(); // 光流速度环控制-20ms
		// LC307_Position_Control(); // 光流位置环控制-40ms

/*
NRF24L01:
*/
		NRF24L01_Data();

/*
printf节拍调试-100ms:
*/	
		if (print_task_flag)
		{
			print_task_flag = 0;
			// usart_printf(USART3,"LC307 Vx:%.3f Vy:%.3f Px:%.3f Py:%.3f Lost:%d\r\n",
			// g_lc307_speed_x,g_lc307_speed_y,g_lc307_pos_x,g_lc307_pos_y,g_lost_pos_dev);
		}

/*
无线串口调试:
*/	
		// USART3_Data();

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
*/
		// TIM1_DShot_Write(0, speed_temp, 0, 0); // 四路同步下发DShot油门

/*
ADC电流环：
*/
		// ADC_Get();
	}
}
