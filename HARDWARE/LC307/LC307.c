#include "LC307.h"
#include "delay.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_usart.h"
#include <string.h>

uint8_t g_lost_pos_dev = 0;

#define LC307_BUFFER_LEN      14U
#define LC307_TIMEOUT_TICKS   84000U

#define LC307_ERR_XOR         1U
#define LC307_ERR_ACK         2U

static uint8_t serialbuffer[LC307_BUFFER_LEN];
static uint8_t LC307_InitFlag = 0;
static uint8_t lc307_rx_index = 0U;

// 光流数据配置表
static const uint8_t tab_BF3901_60hz[] = {
0x12, 0x80, 0x11, 0x30, 0x1b, 0x06, 0x6b, 0x43, 0x12, 0x20, 0x3a, 0x00, 0x15, 0x02, 0x62, 0x81, 0x08, 0xa0, 0x06, 0x68, 0x2b, 0x20, 0x92, 0x25, 0x27, 0x97, 0x17, 0x01, 0x18, 0x79,
0x19, 0x00, 0x1a, 0xa0, 0x03, 0x00, 0x13, 0x00, 0x01, 0x13, 0x02, 0x20, 0x87, 0x16, 0x8c, 0x01, 0x8d, 0xcc, 0x13, 0x07, 0x33, 0x10, 0x34, 0x1d, 0x35, 0x46, 0x36, 0x40, 0x37, 0xa4,
0x38, 0x7c, 0x65, 0x46, 0x66, 0x46, 0x6e, 0x20, 0x9b, 0xa4, 0x9c, 0x7c, 0xbc, 0x0c, 0xbd, 0xa4, 0xbe, 0x7c, 0x20, 0x09, 0x09, 0x03, 0x72, 0x2f, 0x73, 0x2f, 0x74, 0xa7, 0x75, 0x12,
0x79, 0x8d, 0x7a, 0x00, 0x7e, 0xfa, 0x70, 0x0f, 0x7c, 0x84, 0x7d, 0xba, 0x5b, 0xc2, 0x76, 0x90, 0x7b, 0x55, 0x71, 0x46, 0x77, 0xdd, 0x13, 0x0f, 0x8a, 0x10, 0x8b, 0x20, 0x8e, 0x21,
0x8f, 0x40, 0x94, 0x41, 0x95, 0x7e, 0x96, 0x7f, 0x97, 0xf3, 0x13, 0x07, 0x24, 0x58, 0x97, 0x48, 0x25, 0x08, 0x94, 0xb5, 0x95, 0xc0, 0x80, 0xf4, 0x81, 0xe0, 0x82, 0x1b, 0x83, 0x37,
0x84, 0x39, 0x85, 0x58, 0x86, 0xff, 0x89, 0x15, 0x8a, 0xb8, 0x8b, 0x99, 0x39, 0x98, 0x3f, 0x98, 0x90, 0xa0, 0x91, 0xe0, 0x40, 0x20, 0x41, 0x28, 0x42, 0x26, 0x43, 0x25, 0x44, 0x1f,
0x45, 0x1a, 0x46, 0x16, 0x47, 0x12, 0x48, 0x0f, 0x49, 0x0d, 0x4b, 0x0b, 0x4c, 0x0a, 0x4e, 0x08, 0x4f, 0x06, 0x50, 0x06, 0x5a, 0x56, 0x51, 0x1b, 0x52, 0x04, 0x53, 0x4a, 0x54, 0x26,
0x57, 0x75, 0x58, 0x2b, 0x5a, 0xd6, 0x51, 0x28, 0x52, 0x1e, 0x53, 0x9e, 0x54, 0x70, 0x57, 0x50, 0x58, 0x07, 0x5c, 0x28, 0xb0, 0xe0, 0xb1, 0xc0, 0xb2, 0xb0, 0xb3, 0x4f, 0xb4, 0x63,
0xb4, 0xe3, 0xb1, 0xf0, 0xb2, 0xa0, 0x55, 0x00, 0x56, 0x40, 0x96, 0x50, 0x9a, 0x30, 0x6a, 0x81, 0x23, 0x33, 0xa0, 0xd0, 0xa1, 0x31, 0xa6, 0x04, 0xa2, 0x0f, 0xa3, 0x2b, 0xa4, 0x0f,
0xa5, 0x2b, 0xa7, 0x9a, 0xa8, 0x1c, 0xa9, 0x11, 0xaa, 0x16, 0xab, 0x16, 0xac, 0x3c, 0xad, 0xf0, 0xae, 0x57, 0xc6, 0xaa, 0xd2, 0x78, 0xd0, 0xb4, 0xd1, 0x00, 0xc8, 0x10, 0xc9, 0x12,
0xd3, 0x09, 0xd4, 0x2a, 0xee, 0x4c, 0x7e, 0xfa, 0x74, 0xa7, 0x78, 0x4e, 0x60, 0xe7, 0x61, 0xc8, 0x6d, 0x70, 0x1e, 0x39, 0x98, 0x1a, 0x9d, 0xf0
};

#pragma pack(1)
typedef struct
{
    uint8_t head1;
    uint8_t bufcount;
    short flowX;
    short flowY;
    uint16_t timespan;
    uint16_t distance;
    uint8_t quality;
    uint8_t version;
    uint8_t XORSum;
    uint8_t end;
} OpticalFlowFrame_t;
#pragma pack()

static OpticalFlowFrame_t opfRecv_Frame = {0};
static float speed[2] = {0.0f};

__weak void getOpticalFlowResult_Callback(float *buf)
{
    (void)buf;
}

static uint8_t XOR_Checksum(const uint8_t *data, uint16_t length)
{
    uint8_t xor_result = 0U;
    uint16_t i = 0U;
    for (i = 0U; i < length; i++)
    {
        xor_result ^= data[i];
    }
    return xor_result;
}

static void uart1_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

    GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_USART1);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_USART1);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = 19200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);
    USART_Cmd(USART1, ENABLE);
}

static void LC307_SendData(const uint8_t *data, uint16_t len)
{
    uint16_t i = 0U;
    for (i = 0U; i < len; i++)
    {
        USART_SendData(USART1, data[i]);
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
        {
        }
    }
}

static uint8_t LC307_RecvData(uint8_t *data, uint16_t len)
{
    uint16_t i = 0U;
    uint32_t waittime = 0U;

    for (i = 0U; i < len; i++)
    {
        waittime = 0U;
        while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET)
        {
            waittime++;
            if (waittime >= LC307_TIMEOUT_TICKS)
            {
                g_lost_pos_dev = 1U;
                return 0U;
            }
        }

        data[i] = (uint8_t)USART_ReceiveData(USART1);
    }

    return 1U;
}

static void LC307_Callback(uint16_t size)
{
    if ((size == LC307_BUFFER_LEN) && (serialbuffer[0] == 0xFEU))
    {
        if (serialbuffer[12] == XOR_Checksum(&serialbuffer[2], 10U))
        {
            LC307_InitFlag = 1U;
            memcpy(&opfRecv_Frame, serialbuffer, sizeof(OpticalFlowFrame_t));

            speed[0] = (float)opfRecv_Frame.flowX;
            speed[1] = (float)opfRecv_Frame.flowY;

            getOpticalFlowResult_Callback(speed);
        }
    }
}

void Opf_LC307_Start(void)
{
    lc307_rx_index = 0U;
}

uint8_t Opf_LC307_Init(void)
{
    uint8_t step1_initbuf[7] = {0xAA, 0xAB, 0x96, 0x26, 0xBC, 0x50, 0x5C};
    uint8_t feedbackbuf[3] = {0};
    uint16_t i = 0U;

    uart1_init();

    delay_ms(200);

    if (LC307_InitFlag)
    {
        return 0U;
    }
    else
    {
        (void)USART_ReceiveData(USART1);
    }

    LC307_SendData(step1_initbuf, 7U);
    if (!LC307_RecvData(feedbackbuf, 3U))
    {
        return LC307_ERR_ACK;
    }

    if (((uint8_t)(feedbackbuf[0] ^ feedbackbuf[1])) != feedbackbuf[2])
    {
        g_lost_pos_dev = 1U;
        return LC307_ERR_XOR;
    }

    if (feedbackbuf[1] != 0x00U)
    {
        g_lost_pos_dev = 1U;
        return LC307_ERR_ACK;
    }

    for (i = 0U; i < (uint16_t)sizeof(tab_BF3901_60hz); i += 2U)
    {
        uint8_t buf[5] = {0xBB, 0xDC, tab_BF3901_60hz[i], tab_BF3901_60hz[i + 1U], 0U};
        buf[4] = (uint8_t)(buf[1] ^ buf[2] ^ buf[3]);
        LC307_SendData(buf, 5U);

        if (!LC307_RecvData(feedbackbuf, 3U))
        {
            return LC307_ERR_ACK;
        }

        if (((uint8_t)(feedbackbuf[0] ^ feedbackbuf[1])) != feedbackbuf[2])
        {
            g_lost_pos_dev = 1U;
            return LC307_ERR_XOR;
        }

        if (feedbackbuf[1] != 0x00U)
        {
            g_lost_pos_dev = 1U;
            return LC307_ERR_ACK;
        }
    }

    {
        uint8_t closecfg = 0xDD;
        LC307_SendData(&closecfg, 1U);
    }

    LC307_InitFlag = 1U;
    return 0U;
}

void LC307_USART1_IdleHandler(void)
{
}

void LC307_USART1_RxByteHandler(uint8_t data)
{
    if (lc307_rx_index == 0U)
    {
        if (data != 0xFEU)
        {
            return;
        }
    }

    serialbuffer[lc307_rx_index] = data;
    lc307_rx_index++;

    if (lc307_rx_index >= LC307_BUFFER_LEN)
    {
        LC307_Callback(LC307_BUFFER_LEN);
        lc307_rx_index = 0U;
    }
}
