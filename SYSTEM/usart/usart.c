#include "usart.h"

// 单个USART实例对应一套发送环形队列:
// - head: 写入位置(生产者)
// - tail: 发送位置(消费者, 在TXE中断里推进)
typedef struct
{
    USART_TypeDef* instance;
    volatile uint16_t head;
    volatile uint16_t tail;
    uint8_t buf[USART_TX_BUF_SIZE];
} USART_TxAsyncQueue;

static USART_TxAsyncQueue g_usart_tx_q1 = {USART1, 0U, 0U, {0}}; // USART1的异步发送队列
static USART_TxAsyncQueue g_usart_tx_q2 = {USART2, 0U, 0U, {0}}; // USART2的异步发送队列
static USART_TxAsyncQueue g_usart_tx_q3 = {USART3, 0U, 0U, {0}}; // USART3的异步发送队列

// 根据USART实例找到对应的异步发送队列
static USART_TxAsyncQueue* usart_get_tx_queue(USART_TypeDef* USARTx)
{
    if (USARTx == USART1)
    {
        return &g_usart_tx_q1;
    }
    if (USARTx == USART2)
    {
        return &g_usart_tx_q2;
    }
    if (USARTx == USART3)
    {
        return &g_usart_tx_q3;
    }

    return NULL;
}

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
    // 优先走非阻塞路径: 入队成功则立即返回, 不阻塞主循环
    if (usart_send_byte_async(USARTx, Byte) != 0U)
    {
        return;
    }

    // 当队列不可用或队列满时, 退化为阻塞发送, 保证数据尽量不丢
    USART_SendData(USARTx, Byte);
    while (USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET);
}

// 发送一个字节(非阻塞): 数据入环形缓冲, 由TXE中断发送
uint8_t usart_send_byte_async(USART_TypeDef* USARTx, uint8_t Byte)
{
    uint32_t primask;
    uint16_t next_head;
    USART_TxAsyncQueue* q;

    q = usart_get_tx_queue(USARTx);
    if (q == NULL)
    {
        // 非USART1/2/3实例, 不支持异步队列
        return 0U;
    }

    // head/tail由主循环与中断并发访问, 这里短暂关中断保证原子更新
    primask = __get_PRIMASK();
    __disable_irq();

    next_head = (uint16_t)((q->head + 1U) % USART_TX_BUF_SIZE);
    if (next_head == q->tail)
    {
        // 队列满: 返回0告知上层失败(由上层决定是否阻塞发送)
        if (primask == 0U)
        {
            __enable_irq();
        }
        return 0U;
    }

    q->buf[q->head] = Byte;
    q->head = next_head;
    // 使能TXE中断后, 硬件在发送寄存器可写时会触发中断, 中断中出队发送
    USART_ITConfig(q->instance, USART_IT_TXE, ENABLE);

    if (primask == 0U)
    {
        __enable_irq();
    }

    return 1U;
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
    (void)f;
    // printf 的每个字符都会进入这里, 具体输出串口由 PRINTF_USART 宏决定
    usart_send_byte(PRINTF_USART, (uint8_t)ch);
    return ch;
}

// usart_printf 函数
void usart_printf(USART_TypeDef* USARTx, const char *format, ...)
{
    char String[100];
    int len;
    va_list arg;

    va_start(arg, format);
    // 先格式化到本地缓冲, 再统一按字符串发送
    len = vsnprintf(String, sizeof(String), format, arg);
    va_end(arg);

    if (len <= 0)
    {
        return;
    }

    usart_SendString(USARTx, String);
}

// USART发送中断处理函数: 从对应串口环形缓冲区取数据发送
void usart_tx_irq_handler(USART_TypeDef* USARTx)
{
    USART_TxAsyncQueue* q;

    q = usart_get_tx_queue(USARTx);
    if (q == NULL)
    {
        return;
    }

    // 队列非空: 发送tail位置一个字节, 然后tail前移
    if (q->tail != q->head)
    {
        USART_SendData(q->instance, q->buf[q->tail]);
        q->tail = (uint16_t)((q->tail + 1U) % USART_TX_BUF_SIZE);
    }
    else
    {
        // 队列已空: 关闭TXE中断, 避免空中断反复进入
        USART_ITConfig(q->instance, USART_IT_TXE, DISABLE);
    }
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
