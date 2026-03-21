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
		
/*
MPU6050:
*/		
	// mpu_dmp_get_data(&Pitch,&Roll,&Yaw);	    // 读取角度
	// printf("%.2f, %.2f, %.2f\r\n",Pitch, Roll, Yaw);
		
/*
QMC5883L:
*/
	// Angle_XY = QMC_Data();
	// printf("%.2f\r\n",Angle_XY);

	// ControlTask_FusionUpdate(); // 更新融合数据
 	// yaw_fused = ControlTask_GetYawFused(); // 获取融合后的航向角
	
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
//	OLED_Clear();
//	OLED_Printf(0,0,OLED_8X16,"T:%d",Timer_Bsp_t);
//	OLED_Printf(0,16,OLED_8X16,"speed:%d",speed_temp);
//	OLED_Printf(0,32,OLED_8X16,"BMP: %.2f",alt);
//	OLED_Update();
	
	OLED_Clear();
	OLED_Printf(0,0,OLED_8X16,"T:%d",Timer_Bsp_t);
	OLED_Printf(0, 16,  OLED_8X16, "speed:%d", speed_temp);
	OLED_Printf(0, 32, OLED_8X16, "Key:%d", NRF24L01_RxPacket[1]);
	OLED_Update();

	// OLED_Clear();
	// OLED_Printf(0,0,OLED_8X16,"P:%.1f",Pitch);
	// OLED_Printf(64,0,OLED_8X16,"R:%.1f",Roll);
	// OLED_Printf(0,0,OLED_8X16,"FUS:%.1f",yaw_fused);
	// OLED_Printf(0,16,OLED_8X16,"Yaw: %.2f",Yaw);
	// OLED_Printf(0,32,OLED_8X16,"QMC: %.2f",Angle_XY);
	// OLED_Printf(0,48,OLED_8X16,"BMP: %.2f",alt);
	// OLED_Update();

/*
电机调试:
MOS1-上，白-5V
MOS2-下，白-0V
MOS3-下，白-5V
MOS4-上，白-0V
*/
	Motor_Test();
	//printf("speed_temp:%d\r\n",speed_temp);

/*
ADC电流环：
	ADC0-MOS
	ADC1-MOS
	ADC2-MOS
	ADC3-MOS4
*/

	// ADC_Get();

	// OLED_Clear();
	// OLED_Printf(0,0,OLED_8X16,"0: %d",ADValue[0]); // ADC原始值
	// OLED_Printf(0,16,OLED_8X16,"1: %d",ADValue[1]);
	// OLED_Printf(0,32,OLED_8X16,"2: %d",ADValue[2]);
	// OLED_Printf(0,48,OLED_8X16,"3: %d",ADValue[3]);
	
	// OLED_Printf(84,0,OLED_8X16,"%.1f",Voltage[0]); // ADC滤波平均值
	// OLED_Printf(84,16,OLED_8X16,"%.1f",Voltage[1]);
	// OLED_Printf(84,32,OLED_8X16,"%.1f",Voltage[2]);
	// OLED_Printf(84,48,OLED_8X16,"%.1f",Voltage[3]);	
	// OLED_Update();
	}
}
