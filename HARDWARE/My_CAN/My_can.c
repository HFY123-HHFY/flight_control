#include "My_can.h"

void My_CAN_Init(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	/* 注意: F4上CAN2从属于CAN1内核, 使用CAN2时必须同时开启CAN1时钟 */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1 | RCC_APB1Periph_CAN2, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
  	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	//引脚复用映射配置
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource12,GPIO_AF_CAN2); //GPIOB12复用为CAN2_RX
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource13,GPIO_AF_CAN2); //GPIOB13复用为CAN2_TX
	
	CAN_InitTypeDef CAN_InitStructure;
	CAN_StructInit(&CAN_InitStructure);
	CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;
	CAN_InitStructure.CAN_Prescaler = 48;		//波特率 = 36M / 48 / (1 + 2 + 3) = 125K
	CAN_InitStructure.CAN_BS1 = CAN_BS1_2tq;
	CAN_InitStructure.CAN_BS2 = CAN_BS2_3tq;
	CAN_InitStructure.CAN_SJW = CAN_SJW_2tq;
	CAN_InitStructure.CAN_NART = DISABLE;
	CAN_InitStructure.CAN_TXFP = DISABLE;
	CAN_InitStructure.CAN_RFLM = DISABLE;
	CAN_InitStructure.CAN_AWUM = DISABLE;
	CAN_InitStructure.CAN_TTCM = DISABLE;
	CAN_InitStructure.CAN_ABOM = DISABLE;
	CAN_Init(CAN2, &CAN_InitStructure);
	
	CAN_FilterInitTypeDef CAN_FilterInitStructure;
	/* CAN2推荐使用14~27滤波器bank, 这里用14做全通过 */
	CAN_FilterInitStructure.CAN_FilterNumber = 14;
	CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;
	CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;
	CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0;
	CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
	CAN_FilterInit(&CAN_FilterInitStructure);
}

/*
	发送数据
	ID: 标识符
	Length: 数据长度，范围0-8
	Data: 数据内容，长度为Length
*/
void My_CAN_Transmit(uint32_t ID, uint8_t Length, uint8_t *Data)
{
	CanTxMsg TxMessage;
	TxMessage.StdId = ID;
	TxMessage.ExtId = ID;
	TxMessage.IDE = CAN_Id_Standard;		//CAN_ID_STD
	TxMessage.RTR = CAN_RTR_Data;
	TxMessage.DLC = Length;
	for (uint8_t i = 0; i < Length; i ++)
	{
		TxMessage.Data[i] = Data[i];
	}
	
	uint8_t TransmitMailbox = CAN_Transmit(CAN2, &TxMessage);
	
	uint32_t Timeout = 0;
	while (CAN_TransmitStatus(CAN2, TransmitMailbox) != CAN_TxStatus_Ok)
	{
		Timeout ++;
		if (Timeout > 100000)
		{
			break;
		}
	}
}

//接收数据标志位，1表示有数据待接收，0表示没有数据
uint8_t My_CAN_ReceiveFlag(void)
{
	if (CAN_MessagePending(CAN2, CAN_FIFO0) > 0)
	{
		return 1;
	}
	return 0;
}

/*
	接收数据
	ID: 标识符
	Length: 数据长度，范围0-8
	Data: 数据内容，长度为Length
*/
void My_CAN_Receive(uint32_t *ID, uint8_t *Length, uint8_t *Data)
{
	CanRxMsg RxMessage;
	CAN_Receive(CAN2, CAN_FIFO0, &RxMessage);
	
	if (RxMessage.IDE == CAN_Id_Standard)
	{
		*ID = RxMessage.StdId;
	}
	else
	{
		*ID = RxMessage.ExtId;
	}
	
	if (RxMessage.RTR == CAN_RTR_Data)
	{
		*Length = RxMessage.DLC;
		for (uint8_t i = 0; i < *Length; i ++)
		{
			Data[i] = RxMessage.Data[i];
		}
	}
	else
	{
		//...
	}
}

static uint32_t RxID = 0; //接收ID
static uint8_t RxLength = 0; //接收报文长度
static uint8_t RxData[8] = {0}; //接收数据

uint8_t My_Can_RX_Flag = 0; //接收标志位，1表示有数据待接收，0表示没有数据

void My_Can_TX_Data(void)
{
	static uint32_t TxID = 0x666; //发送ID
	static uint8_t TxLength = 4; //发送报文长度
	static uint8_t TxData[8] = {0x11, 0x22, 0x33, 0x44}; //发送数据

	if (USART_3_RX == 'a')
	{
		USART_3_RX = 0; // 清除接收标志
		TxData[0] ++;
		TxData[1] ++;
		TxData[2] ++;
		TxData[3] ++;
		My_CAN_Transmit(TxID, TxLength, TxData); // 发送数据
	}
}

void My_Can_RX_Data(void)
{
	My_Can_RX_Flag = My_CAN_ReceiveFlag();

	if (My_Can_RX_Flag)
	{
		My_CAN_Receive(&RxID, &RxLength, RxData); // 接收数据
		usart_printf(USART3, "Flag: %d,ID=0x%X, L:%d, Data: 0x%X, 0x%X, 0x%X, 0x%X\r\n", My_Can_RX_Flag, RxID, RxLength, RxData[0], RxData[1], RxData[2], RxData[3]);
	}
}

