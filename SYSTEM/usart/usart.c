#include "usart.h"

#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
// Keil/ARMCC 下关闭半主机输出
#pragma import(__use_no_semihosting)
struct __FILE
{
    int handle;
};

FILE __stdout;

void _sys_exit(int x)
{
    (void)x;
}
#elif defined(__GNUC__)
int _write(int file, char *ptr, int len)
{
    int index;

    (void)file;
    for (index = 0; index < len; index++)
    {
        usart_send_byte(USART2, (uint8_t)ptr[index]);
    }
    return len;
}
#endif

/*或者
//重定义fputc函数 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
    USART1->DR = (u8) ch;      
	return ch;
}
*/

// 发送一个字节
void usart_send_byte(USART_TypeDef* USARTx, uint8_t Byte)
{
    USART_SendData(USARTx, Byte);
    while (USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET);
}

// 发送一个字符串
void usart_SendString(USART_TypeDef* USARTx, const char *String)
{
    uint8_t i;
    for (i = 0; String[i] != '\0'; i++)
    {
        usart_send_byte(USARTx, String[i]);
    }
}

// 发送一个数字
void usart_send_number(USART_TypeDef* USARTx, uint32_t Number)
{
    char String[10];
    sprintf(String, "%lu", Number);
    usart_SendString(USARTx, String);
}

// 次方函数
uint32_t usart_pow(uint32_t X, uint32_t Y)
{
    uint32_t Result = 1;
    while (Y--)
    {
        Result *= X;
    }
    return Result;
}

// 发送一个数组
void usart_send_array(USART_TypeDef* USARTx, uint8_t *Array, uint16_t Length)
{
    uint16_t i;
    for (i = 0; i < Length; i++)
    {
        usart_send_byte(USARTx, Array[i]);
    }
}

// 重定向 C 库函数 printf 到串口
int fputc(int ch, FILE *f)
{
    usart_send_byte(USART3, (uint8_t) ch); // 这里使用 USART3
    return ch;
}

// usart_printf 函数
void usart_printf(USART_TypeDef* USARTx, const char *format, ...)
{
    char String[100];
    va_list arg;
    va_start(arg, format);
    vsprintf(String, format, arg);
    va_end(arg);
    usart_SendString(USARTx, String);
}

USART_DataType USART_DataTypeStruct;

// 数据包解析函数
void usart_Dispose_Data(USART_TypeDef* USARTx, USART_DataType* USART_DataTypeStruct, uint8_t RxData)
{
    switch (USART_DataTypeStruct->state)
    {
        case 0: // 空闲状态,检测包头
            if (RxData == 's')  // 包头
            {
                USART_DataTypeStruct->state = 1;       // 进入接收状态
                USART_DataTypeStruct->current_index = 0; // 初始化数据索引
                USART_DataTypeStruct->buffer_len = 0;    // 清空缓冲区
                memset(USART_DataTypeStruct->buffer, 0, sizeof(USART_DataTypeStruct->buffer));//初始化缓冲区
            }
            break;

        case 1: // 接收数据中
            if (RxData == 'e')  // 包尾
            {
                // 处理最后一个数据
                if (USART_DataTypeStruct->buffer_len > 0)
                {
                    int16_t value = 0;
                    uint8_t i = 0;
                    uint8_t is_negative = 0;
                    // 判断负号
                    if (USART_DataTypeStruct->buffer[0] == '-')
                    {
                        is_negative = 1;
                        i = 1;
                    }
                    for (; i < USART_DataTypeStruct->buffer_len; i++)
                    {
                        if (USART_DataTypeStruct->buffer[i] >= '0' && USART_DataTypeStruct->buffer[i] <= '9')
                        {
                            value = value * 10 + (USART_DataTypeStruct->buffer[i] - '0');
                        }
                        else
                        {
                            USART_DataTypeStruct->state = 0;
                            break;
                        }
                    }
                    if (is_negative) value = -value;
                    if (USART_DataTypeStruct->current_index < Data_len)
                    {
                        USART_DataTypeStruct->data[USART_DataTypeStruct->current_index] = value;
                        USART_DataTypeStruct->count = USART_DataTypeStruct->current_index + 1;
                    }
                }
                USART_DataTypeStruct->state = 2; // 接收完成
            }
            else if (RxData == ',')  // 分隔符
            {
                if (USART_DataTypeStruct->buffer_len > 0)
                {
                    int16_t value = 0;
                    uint8_t i = 0;
                    uint8_t is_negative = 0;
                    if (USART_DataTypeStruct->buffer[0] == '-')
                    {
                        is_negative = 1;
                        i = 1;
                    }
                    for (; i < USART_DataTypeStruct->buffer_len; i++)
                    {
                        if (USART_DataTypeStruct->buffer[i] >= '0' && USART_DataTypeStruct->buffer[i] <= '9')
                        {
                            value = value * 10 + (USART_DataTypeStruct->buffer[i] - '0');
                        }
                        else
                        {
                            USART_DataTypeStruct->state = 0;
                            break;
                        }
                    }
                    if (is_negative) value = -value;
                    if (USART_DataTypeStruct->current_index < Data_len)
                    {
                        USART_DataTypeStruct->data[USART_DataTypeStruct->current_index] = value;
                        USART_DataTypeStruct->current_index++;
                    }
                    USART_DataTypeStruct->buffer_len = 0;
                    memset(USART_DataTypeStruct->buffer, 0, sizeof(USART_DataTypeStruct->buffer));
                }
            }
            else if ((RxData >= '0' && RxData <= '9') || RxData == '-')  // 数字或负号
            {
                if (USART_DataTypeStruct->buffer_len < 15)
                {
                    // 只允许负号出现在第一个字符
                    if (RxData == '-' && USART_DataTypeStruct->buffer_len != 0)
                    {
                        USART_DataTypeStruct->state = 0; // 非法负号，重置
                    }
                    else
                    {
                        USART_DataTypeStruct->buffer[USART_DataTypeStruct->buffer_len++] = RxData;
                    }
                }
                else
                {
                    USART_DataTypeStruct->state = 0; // 缓冲区溢出，重置状态
                }
            }
            else
            {
                USART_DataTypeStruct->state = 0; // 其他无效字符，重置状态
            }
            break;

        case 2: // 接收完成状态
            if (RxData == 's') // 新包头
            {
                USART_DataTypeStruct->state = 1;
                USART_DataTypeStruct->current_index = 0;
                USART_DataTypeStruct->count = 0;
                USART_DataTypeStruct->buffer_len = 0;
                memset(USART_DataTypeStruct->buffer, 0, sizeof(USART_DataTypeStruct->buffer));
            }
            break;
    }
}

// 获取数据函数
int16_t USART_Deal(USART_DataType* pData, int8_t index)
{
    if (pData == NULL || index >= pData->count) 
    {
        return 0; // 返回0表示无效索引
    }
    return pData->data[index];
}

//刷新数据
/*
void usart_refresh(void)
{
    if (USART_DataTypeStruct.state == 2) 
    {
		data1 = USART_Deal(&USART_DataTypeStruct, 0); // 获取第1个数据
		data2 = USART_Deal(&USART_DataTypeStruct, 1); // 获取第2个数据
		data3 = USART_Deal(&USART_DataTypeStruct, 2); // 获取第3个数据
		data4 = USART_Deal(&USART_DataTypeStruct, 3); // 获取第4个数据
		USART_DataTypeStruct.state = 0; // 重置状态以接收新数据
        usart_printf(USART3, "fff\r\n");
    }
}
*/
