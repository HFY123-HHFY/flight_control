#ifndef __MY_I2C_H
#define __MY_I2C_H

#include "stm32f4xx.h" 
#include "delay.h"

/* I2C引脚定义 */
#define My_I2C_PORT GPIOB
#define My_I2C_SCL GPIO_Pin_8
#define My_I2C_SDA GPIO_Pin_9

/* 引脚操作层函数声明 */
void MyI2C_W_SCL(uint8_t BitValue); // 写SCL
void MyI2C_W_SDA(uint8_t BitValue); // 写SDA
uint8_t MyI2C_R_SDA(void); // 读SDA

void MyI2C_Set_SDA_Input(void); // 设置SDA为输入模式
void MyI2C_Set_SDA_Output(void); // 设置SDA为输出模式
/*
//IO方向设置
#define SDA_IN()  {GPIOB->MODER&=~(3<<(9*2));GPIOB->MODER|=0<<9*2;}	//PB9输入模式
#define SDA_OUT() {GPIOB->MODER&=~(3<<(9*2));GPIOB->MODER|=1<<9*2;} //PB9输出模式
*/

/* 函数声明 */
void MyI2C_Init(void);
void MyI2C_Start(void); // I2C起始
void MyI2C_Stop(void); // I2C停止
void MyI2C_SendByte(uint8_t Byte); // I2C发送一个字节
uint8_t MyI2C_ReceiveByte(unsigned char Ack); // I2C接收一个字节
void MyI2C_Ack(void); // 发送ACK应答
void MyI2C_NAck(void); // 发送NACK应答
uint8_t MyI2C_Wait_Ack(void); // 等待应答信号

#endif
