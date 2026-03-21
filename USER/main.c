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
	mpu_angle(); // 读取MPU6050角度数据
	PID_Pitch_Roll_Combined(Pitch, Roll); // Pitch 和 Roll 合并双环控制函数
	
	Set_PID(&pid_pitch, 5.0f, 0.00f, 1.0f); // 设置pitch环的PID参数
	Set_PID(&pid_roll, 5.0f, 0.00f, 1.0f); // 设置roll环的PID参数

	// printf("speed_temp: %d, pitch_out: %.2f, roll_out: %.2f, OUT: %.2f\r\n", 
	// 	speed_temp, pid_pitch.output, pid_roll.output, speed_temp + pid_pitch.output + pid_roll.output);
	// printf("Pitch: %.2f, out :%.2f, Kp: %.2f            Roll: %.2f, out :%.2f, Kp: %.2f\r\n",
	// Pitch, pid_pitch.output , pid_pitch.kp, Roll, pid_roll.output , pid_roll.kp);
	// printf("Pitch: %.2f, Roll :%.2f, Yaw: %.2f, Fused: %.2f\r\n",Pitch, Roll, Yaw, yaw_fused);
		
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
	OLED_Clear();
	OLED_Printf(0,0,OLED_8X16,"USART_3_RX:%d",USART_3_RX);
//	OLED_Printf(0,0,OLED_8X16,"T:%d",Timer_Bsp_t);
//	OLED_Printf(0,16,OLED_8X16,"speed:%d",speed_temp);
//	OLED_Printf(0,32,OLED_8X16,"BMP: %.2f",alt);
	OLED_Update();

	if (uart3_flag == 1)
	{
		uart3_flag = 0;
		float new_kp, new_ki, new_kd;
		// 更新Pitch环
        Set_PID(&pid_pitch, new_kp, new_ki, new_kd); 
        // 更新Roll环
        Set_PID(&pid_roll, new_kp, new_ki, new_kd);  
	}
	
	
	// OLED_Clear();
	// OLED_Printf(0,0,OLED_8X16,"T:%d",Timer_Bsp_t);
	// OLED_Printf(0, 16,  OLED_8X16, "speed:%d", speed_temp);
	// OLED_Printf(0, 32, OLED_8X16, "Key:%d", NRF24L01_RxPacket[1]);
	// OLED_Update();

	// OLED_Clear();
	// OLED_Printf(0,0,OLED_8X16,"T:%d",Timer_Bsp_t);
	// OLED_Printf(0,16,OLED_8X16,"Pitch:%.2f",Pitch);
	// OLED_Printf(0,32,OLED_8X16,"Roll:%.2f",Roll);
	// OLED_Printf(0,48,OLED_8X16,"%.2f",pid_pitch.output);
	// OLED_Printf(64,48,OLED_8X16,"%.2f",pid_roll.output);
	// OLED_Printf(0,16,OLED_8X16,"Yaw: %.2f",Yaw);
	// OLED_Printf(0,32,OLED_8X16,"QMC: %.2f",Angle_XY);
	// OLED_Printf(0,48,OLED_8X16,"FUS:%.2f",yaw_fused);
	// OLED_Printf(0,16,OLED_8X16,"Yaw: %.2f",Yaw);
	// OLED_Printf(0,48,OLED_8X16,"BMP: %.2f",alt);
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
