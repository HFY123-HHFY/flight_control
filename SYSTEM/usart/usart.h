#ifndef __USART_H
#define __USART_H

#include "stm32f4xx.h" 
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#ifndef USART_TX_BUF_SIZE // USART异步发送缓冲区大小(单位:字节)
#define USART_TX_BUF_SIZE 512U // 存储512个待发送的字节数据
#endif

#ifndef PRINTF_USART // printf使用的USART, 默认为USART3
#define PRINTF_USART USART3
#endif

void usart_send_byte(USART_TypeDef* USARTx, uint8_t Byte);//发送一个字节
uint8_t usart_send_byte_async(USART_TypeDef* USARTx, uint8_t Byte);//发送一个字节(非阻塞)
void usart_SendString(USART_TypeDef* USARTx, const char *String);//发送一个字符串
void usart_send_number(USART_TypeDef* USARTx, uint32_t Number);//发送一个数字
uint32_t usart_pow(uint32_t X, uint32_t Y);//次方函数
void usart_send_array(USART_TypeDef* USARTx, uint8_t *Array, uint16_t Length);//发送一个数组
int fputc(int ch, FILE *f);//重定向 C 库函数 printf 到串口
void usart_printf(USART_TypeDef* USARTx, const char *format, ...);//usart_printf 函数
void usart_tx_irq_handler(USART_TypeDef* USARTx); // USART发送中断处理(非阻塞发送)

#define Data_len 10//数据长度

typedef struct 
{
    uint16_t data[Data_len];       // 存储数据
    uint8_t count;           // 当前有效数据个数
    uint8_t state;           // 0:空闲 1:接收中 2:接收完成
    uint8_t current_index;   // 当前正在处理的数据索引
    uint8_t buffer[16];      // 临时数据缓冲区
    uint8_t buffer_len;      // 当前缓冲区数据长度
} USART_DataType;

extern USART_DataType USART_DataTypeStruct;
 
// 数据包解析函数
void usart_Dispose_Data(USART_TypeDef* USARTx, USART_DataType* USART_DataTypeStruct, uint8_t RxData);

// 获取数据函数
int16_t USART_Deal(USART_DataType* pData, int8_t index);

#endif
