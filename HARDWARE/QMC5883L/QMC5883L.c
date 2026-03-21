#include "QMC.h"

float Angle_XY = 0;

/**
  * 函    数：qmc写寄存器
  * 参    数：RegAddress 寄存器地址，范围：参考手册的寄存器描述
  * 参    数：Data 要写入寄存器的数据，范围：0x00~0xFF
  * 返 回 值：无
  */
void QMC_WriteReg(uint8_t RegAddress, uint8_t Data)
{
	MyI2C_Start();						//I2C起始
	MyI2C_SendByte(QMC5883L_ADDRESS);	//发送从机地址，读写位为0，表示即将写入
	MyI2C_Wait_Ack();					//接收应答
	MyI2C_SendByte(RegAddress);			//发送寄存器地址
	MyI2C_Wait_Ack();					//接收应答
	MyI2C_SendByte(Data);				//发送要写入寄存器的数据
	MyI2C_Wait_Ack();					//接收应答
	MyI2C_Stop();						//I2C终止
}

/**
  * 函    数：qmc读寄存器
  * 参    数：RegAddress 寄存器地址，范围：参考MPU6050手册的寄存器描述
  * 返 回 值：读取寄存器的数据，范围：0x00~0xFF
  */
uint8_t QMC_ReadReg(uint8_t RegAddress)
{
	uint8_t Data;
	
	MyI2C_Start();						//I2C起始
	MyI2C_SendByte(QMC5883L_ADDRESS);	//发送从机地址，读写位为0，表示即将写入
	MyI2C_Wait_Ack();					//接收应答
	MyI2C_SendByte(RegAddress);			//发送寄存器地址
	MyI2C_Wait_Ack();					//接收应答
	
	MyI2C_Start();						//I2C重复起始
	MyI2C_SendByte(QMC5883L_ADDRESS | 0x01);	//发送从机地址，读写位为1，表示即将读取
	MyI2C_Wait_Ack();					//接收应答
	Data = MyI2C_ReceiveByte(0);			//接收指定寄存器的数据
	MyI2C_NAck();					//发送应答，给从机非应答，终止从机的数据输出
	MyI2C_Stop();						//I2C终止
	
	return Data;
}

uint8_t QMC_GetID(void)
{
	return QMC_ReadReg(0x0D);       //MPU6050_ReadReg(0x0D);		//返回WHO_AM_I寄存器的值
}

void QMC_Init(void)
{
	uint8_t QMC_ID = 0;
	QMC_ID = QMC_GetID();
	printf("ID: 0x%X\r\n",QMC_ID); // 0xFF
	
	QMC_WriteReg(0x09,0x0d);  //控制寄存器配置
	QMC_WriteReg(0x0b,0x01);  //设置清除时间寄存器
//	MyI2C_WriteReg(0x20,0x40);
//	MyI2C_WriteReg(0x21,0x01);
	
}
void QMC_GetData(int16_t *AccX, int16_t *AccY, int16_t *AccZ)
{
	uint8_t DataH, DataL;								//定义数据高8位和低8位的变量
	
	DataH = QMC_ReadReg(QMC5883P_REG_XOUT_H);		//读取加速度计X轴的高8位数据
	DataL = QMC_ReadReg(QMC5883P_REG_XOUT_L);		//读取加速度计X轴的低8位数据
	*AccX = (DataH << 8) | DataL;						//数据拼接，通过输出参数返回
	
	DataH = QMC_ReadReg(QMC5883P_REG_YOUT_H);		//读取加速度计Y轴的高8位数据
	DataL = QMC_ReadReg(QMC5883P_REG_YOUT_L);		//读取加速度计Y轴的低8位数据
	*AccY = (DataH << 8) | DataL;						//数据拼接，通过输出参数返回
	
	DataH = QMC_ReadReg(QMC5883P_REG_ZOUT_H);		//读取加速度计Z轴的高8位数据
	DataL = QMC_ReadReg(QMC5883P_REG_ZOUT_L);		//读取加速度计Z轴的低8位数据
	*AccZ = (DataH << 8) | DataL;						//数据拼接，通过输出参数返回
}

float QMC_Data(void)
{
		int16_t X = 0, Y = 0, Z = 0;
		float Angle_XY_temp = 0;
	
		QMC_GetData(&X,&Y, &Z);
		if(X>0x7fff)X-=0xffff;	  
		if(Y>0x7fff)Y-=0xffff;
		if(Z>0x7fff)Z-=0xffff;	 

		Angle_XY_temp = atan2((double)Y,(double)X) * (180 / 3.14159265) + 180; //计算XY平面角度
		return Angle_XY_temp;
}
