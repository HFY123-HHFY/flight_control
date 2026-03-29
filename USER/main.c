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
MPU6050:
*/
		mpu_angle(); // 读取MPU6050数据
		PID_Pitch_Roll_Combined(Pitch, Roll); // Pitch 和 Roll 合并双环控制函数
		usart_printf(USART3,"Pitch:%.1f, Roll:%.1f, Yaw:%.1f        gyrox:%hd, gyroy:%hd, gyroz:%hd,\r\n",
		Pitch,Roll,Yaw,gyrox,gyroy,gyroz);
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
		USART3_Data();

/*
OLED:
*/
		// OLED_Clear();
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
