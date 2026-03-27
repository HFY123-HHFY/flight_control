#include "Bsp.h"

/*
407_飞控
*/

int main(void)                                                                      
{
	Bsp_Init();

	float R_kp = 0, R_ki = 0, R_kd = 0; // Roll正侧PID参数
	float R_kp_n = 0, R_ki_n = 0, R_kd_n = 0; // Roll负侧PID参数

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
			// ROLL < 0
			case 'a':
				R_kp += 0.1f;
				break;
			
			case 'b':
				R_kp -= 0.1f;
				if (R_kp < 0.0f)
				{
					R_kp = 0.0f;
				}
				break;

			case 'c':
				R_ki += 0.002f;
				break;
			
			case 'd':
				R_ki -= 0.002f;
				if (R_ki < 0.0f)
				{
					R_ki = 0.0f;
				}
				break;

			case 'e':
				R_kd += 0.1f;
				break;
			
			case 'f':
				R_kd -= 0.1f;
				if (R_kd < 0.0f)
				{
					R_kd = 0.0f;
				}
				break;

			// ROLL >= 0
			case 'g':
				R_kp_n += 0.1f;
				break;

			case 'h':
				R_kp_n -= 0.1f;
				if (R_kp_n < 0.0f)
				{
					R_kp_n = 0.0f;
				}
				break;

			case 'i':
				R_ki_n += 0.002f;
				break;

			case 'j':
				R_ki_n -= 0.002f;
				if (R_ki_n < 0.0f)
				{
					R_ki_n = 0.0f;
				}
				break;

			case 'k':
				R_kd_n += 0.1f;
				break;

			case 'l':
				R_kd_n -= 0.1f;
				if (R_kd_n < 0.0f)
				{
					R_kd_n = 0.0f;
				}
				break;

			default:
				break;
			}
			// 更新Roll环
			Set_Roll_BiPID(R_kp, R_ki, R_kd, R_kp_n, R_ki_n, R_kd_n);
		}
		// usart_printf(USART3, "RX:%c\r\n", USART_3_RX);
		// usart_printf(USART3,"T:%d, R:%.1f, P+(%.1f,%.2f,%.1f), P-(%.1f,%.2f,%.1f), out:%.1f\r\n",Timer_Bsp_t, Roll, R_kp, R_ki, R_kd, R_kp_n, R_ki_n, R_kd_n, pid_roll.output);

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
