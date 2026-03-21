#include "My_I2C.h"

/* 引脚操作层 */
void MyI2C_W_SCL(uint8_t BitValue) // 写SCL
{
    GPIO_WriteBit(My_I2C_PORT, My_I2C_SCL, (BitAction)BitValue);
    delay_us(5);
}

void MyI2C_W_SDA(uint8_t BitValue) // 写SDA
{
    GPIO_WriteBit(My_I2C_PORT, My_I2C_SDA, (BitAction)BitValue);
    delay_us(5);
}

uint8_t MyI2C_R_SDA(void) // 读SDA
{
    delay_us(5);
    return GPIO_ReadInputDataBit(My_I2C_PORT, My_I2C_SDA);
}

void MyI2C_Set_SDA_Input(void) // 设置SDA为输入模式
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = My_I2C_SDA;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(My_I2C_PORT, &GPIO_InitStructure);
}

void MyI2C_Set_SDA_Output(void) // 设置SDA为输出模式
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = My_I2C_SDA;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(My_I2C_PORT, &GPIO_InitStructure);
}

/* 初始化层 */
void MyI2C_Init(void)
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE); // 使能GPIOB时钟

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = My_I2C_SCL | My_I2C_SDA;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; // 普通输出模式
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; // 推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; // 100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; // 上拉
    GPIO_Init(My_I2C_PORT, &GPIO_InitStructure);

    MyI2C_W_SCL(1);
    MyI2C_W_SDA(1);
}

/* 协议层 */
void MyI2C_Start(void) // I2C起始
{
    MyI2C_Set_SDA_Output();
    MyI2C_W_SDA(1);
    MyI2C_W_SCL(1);
    delay_us(4);
    MyI2C_W_SDA(0);
    delay_us(4);
    MyI2C_W_SCL(0);//钳住I2C总线，准备发送或接收数据 
}

void MyI2C_Stop(void) // I2C停止
{
  MyI2C_Set_SDA_Output();
  MyI2C_W_SCL(0);
  MyI2C_W_SDA(0);// STOP: 当SCL为高时，SDA从低变高
  delay_us(4);
  MyI2C_W_SCL(1);
  MyI2C_W_SDA(1);
  delay_us(4);
}

//等待应答信号到来
//返回值：1，接收应答失败
//        0，接收应答成功
uint8_t MyI2C_Wait_Ack(void) // 等待应答信号到来
{
    uint8_t ErrTime = 0;
    MyI2C_Set_SDA_Input();
    MyI2C_W_SDA(1);
    delay_us(1);
    MyI2C_W_SCL(1);
    delay_us(1);
    while (MyI2C_R_SDA())
    {
        ErrTime++;
        if (ErrTime > 250)
        {
            MyI2C_Stop();
            return 1; // 超时未响应
        }
    }
    MyI2C_W_SCL(0);
    return 0; // 应答成功
}

void MyI2C_Ack(void) // 发送ACK应答
{
		MyI2C_W_SCL(0);
    MyI2C_Set_SDA_Output();
    MyI2C_W_SDA(0);
    delay_us(2);
    MyI2C_W_SCL(1);
    delay_us(2);
    MyI2C_W_SCL(0);
}

void MyI2C_NAck(void) // 发送NACK应答
{
		MyI2C_W_SCL(0);
    MyI2C_Set_SDA_Output();
    MyI2C_W_SDA(1);
    delay_us(2);
    MyI2C_W_SCL(1);
    delay_us(2);
    MyI2C_W_SCL(0);
}

void MyI2C_SendByte(uint8_t Byte) // I2C发送一个字节
{
    MyI2C_Set_SDA_Output();
    for (uint8_t i = 0; i < 8; i++)
    {
        MyI2C_W_SDA((Byte & 0x80) >> 7);
        Byte <<= 1;
        delay_us(2);
        MyI2C_W_SCL(1);
        delay_us(2);
        MyI2C_W_SCL(0);
        delay_us(2);
    }
}

/***********************************************************
*	I2C 写一个字节
*	Ack : 1 发送ACK 0 发送NACK
*	返回    : 接收到的字节
***********************************************************/
uint8_t MyI2C_ReceiveByte(unsigned char Ack) // I2C接收一个字节
{
    unsigned char i, Byte = 0;
    MyI2C_Set_SDA_Input();
    for (i = 0; i < 8; i++)
    {
        MyI2C_W_SCL(0);
        delay_us(2);
        MyI2C_W_SCL(1);
        Byte <<= 1;
        if (MyI2C_R_SDA())
        {
            Byte++;
        }
        delay_us(1);
    }
    if (Ack)
    {
        MyI2C_Ack();
    }
    else
    {
        MyI2C_NAck();
    }
    return Byte;
}
