#include "LC307.h"

#ifndef __weak
#if defined(__GNUC__)
#define __weak __attribute__((weak))
#else
#define __weak
#endif
#endif

// 光流模块丢失位置设备标志位，0表示正常，1表示丢失
uint8_t g_lost_pos_dev = 0;

// 存放光流原始数据的长度和数据
#define BUFFER_LEN 14
static uint8_t serialbuffer[BUFFER_LEN];

// 光流模块错误码
#define xor_err 1
#define ack_err 2

// 光流模块初始化标志位
static uint8_t LC307_InitFlag = 0;

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

// ============================
// 串口和 DMA 控制层
// ============================

// 配置 USART1 的中断模式：LC307 使用 IDLE 中断 + DMA，关闭 RXNE 中断避免冲突
static void uart1_IT_enableConfig(uint8_t en)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	FunctionalState SetState = ENABLE;

	if (en == 0U) 
    {
		SetState = DISABLE;
	}

	USART_ITConfig(USART1, USART_IT_IDLE, SetState);
	USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);

	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = SetState;
	NVIC_Init(&NVIC_InitStructure);
}

// 配置 USART1_RX 对应的 DMA2 Stream2（单次模式），用于每帧 14 字节搬运
static void uart1_dma_enableConfig(uint8_t en)
{
	// 关闭 DMA2 Stream2 并等待硬件确认关闭
	DMA2_Stream2->CR &= ~DMA_SxCR_EN;
	while ((DMA2_Stream2->CR & DMA_SxCR_EN) != 0U) 
    {

	}

	// 清理 Stream2 的所有中断状态
	DMA2->LIFCR = DMA_LIFCR_CFEIF2 | DMA_LIFCR_CDMEIF2 | DMA_LIFCR_CTEIF2 |
	              DMA_LIFCR_CHTIF2 | DMA_LIFCR_CTCIF2;

	// 关闭时只需要关 USART DMA 请求
	if (en == 0U) 
    {
		USART1->CR3 &= ~USART_CR3_DMAR;
		return;
	}

	DMA2_Stream2->PAR = (uint32_t)&(USART1->DR);
	DMA2_Stream2->M0AR = (uint32_t)serialbuffer;
	DMA2_Stream2->NDTR = BUFFER_LEN;

	// CH4 + 外设到内存 + 内存地址递增 + 高优先级 + 单次模式
	DMA2_Stream2->CR = (4U << 25) | DMA_SxCR_MINC | DMA_SxCR_PL_1;
	DMA2_Stream2->FCR = 0U;

	USART1->CR3 |= USART_CR3_DMAR;
	DMA2_Stream2->CR |= DMA_SxCR_EN;
}

// 向 LC307 发送命令，逐字节阻塞发送，适合初始化配置阶段使用
static void LC307_SendData(uint8_t *data, uint16_t len)
{
	uint16_t i;
	for (i = 0; i < len; i++) 
    {
		USART_SendData(USART1, data[i]);
		while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
        {

		}
	}
}

// 阻塞读取 LC307 返回数据（用于上电配置阶段）
// 这里用于等待模块返回 ACK/应答帧，不适合放在实时循环里
static void LC307_RecvData(uint8_t *data, uint16_t len)
{
	uint32_t waittime = 0;
	uint16_t i;

	for (i = 0; i < len; i++) 
    {
		while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET) 
        {
			waittime++;
			if (waittime >= 84000U) 
            {
				g_lost_pos_dev = 1;
				return;
			}
		}
		data[i] = (uint8_t)USART_ReceiveData(USART1);
		waittime = 0;
	}
}

// 重新装载 DMA 接收缓存并启动下一帧接收
// IDLE 中断进来后会停 DMA、处理当前帧，然后调用这里准备下一帧
static void start_dma_recv(void)
{
	DMA2_Stream2->CR &= ~DMA_SxCR_EN;
	while ((DMA2_Stream2->CR & DMA_SxCR_EN) != 0U) 
    {
        
	}
	DMA2->LIFCR = DMA_LIFCR_CFEIF2 | DMA_LIFCR_CDMEIF2 | DMA_LIFCR_CTEIF2 |
	              DMA_LIFCR_CHTIF2 | DMA_LIFCR_CTCIF2;
	DMA2_Stream2->NDTR = BUFFER_LEN;
	DMA2_Stream2->M0AR = (uint32_t)serialbuffer;
	DMA2_Stream2->CR |= DMA_SxCR_EN;
}

// 对 payload 执行异或校验
// LC307 协议中，部分数据包用 XOR 作为简单校验
static uint8_t XOR_Checksum(const uint8_t *data, uint16_t length)
{
	uint8_t xor_result = 0;
	uint16_t i;

	for (i = 0; i < length; i++) 
    {
		xor_result ^= data[i];
	}
	return xor_result;
}

// ============================
// 光流协议帧定义
// ============================

// 1帧光流数据内容
#pragma pack(1)
typedef struct {
	uint8_t head1;
	uint8_t bufcount;
	int16_t flowX;
	int16_t flowY;
	uint16_t timespan;
	uint16_t distance;
	uint8_t quality;
	uint8_t version;
	uint8_t XORSum;
	uint8_t end;
} OpticalFlowFrame_t;
#pragma pack()

// 解析后的单帧数据缓存，保存最近一次收到的有效光流帧
static OpticalFlowFrame_t opfRecv_Frame = {0};

// ============================
// 光流结果缓存与对外接口
// ============================

// 光流任务节拍标志位
uint8_t lc307_speed_task_flag = 0; // 20ms：速度环节拍
uint8_t lc307_pos_task_flag = 0; // 40ms：位置环节拍

// 光流解算后的数据
float g_lc307_speed_x = 0.0f; // 机体 X 方向速度，单位 m/s，已经做了比例换算和补偿
float g_lc307_speed_y = 0.0f; // 机体 Y 方向速度，单位 m/s，已经做了比例换算和补偿

// 光流积分位置
float g_lc307_pos_x = 0.0f; // 机体 X 方向累计位移，单位 m，建议通过 LC307_UpdatePosition() 定期积分更新
float g_lc307_pos_y = 0.0f; // 机体 Y 方向累计位移，单位 m，建议通过 LC307_UpdatePosition() 定期积分更新

static float use_distance = 1.0f; // 当前飞行高度/测距值，单位米，建议通过 LC307_SetHeight() 定期更新，用于速度换算和补偿
static float gx_filtered = 0.0f; // 角速度补偿量，单位 dps，范围建议 -2 ~ +2，建议通过 LC307_SetGyroCompensation() 定期更新，用于修正机体转动对光流的影响
static float gy_filtered = 0.0f; // 角速度补偿量，单位 dps，范围建议 -2 ~ +2，建议通过 LC307_SetGyroCompensation() 定期更新，用于修正机体转动对光流的影响

// 积分内部状态：用于梯形积分
static float g_lc307_speed_x_prev = 0.0f; // 上一周期的 X 方向速度，用于梯形积分计算
static float g_lc307_speed_y_prev = 0.0f; // 上一周期的 Y 方向速度，用于梯形积分计算

// 限幅函数，限制补偿量在合理范围内，避免过度补偿导致数据异常
static float LC307_Clamp(float value, float min_value, float max_value)
{
	if (value < min_value) 
    {
		return min_value;
	}
	if (value > max_value) 
    {
		return max_value;
	}
	return value;
}

// 设置当前飞行高度/测距值，单位是米，用于光流速度换算和尺度补偿
void LC307_SetHeight(float distance)
{
	use_distance = distance;
}

// 设置角速度补偿量，单位是 dps，范围建议 -2 ~ +2
// 外部一般传入已经滤波后的角速度分量
void LC307_SetGyroCompensation(float gx, float gy)
{
	gx_filtered = gx;
	gy_filtered = gy;
}

// 位置积分更新：由速度积分得到位移，建议放在固定周期任务中调用
void LC307_UpdatePosition(float dt_s)
{
	if (dt_s <= 0.0f) 
    {
		return;
	}

	// 光流异常时不积分，避免错误累积
	if (g_lost_pos_dev != 0U) 
    {
		g_lc307_speed_x_prev = g_lc307_speed_x;
		g_lc307_speed_y_prev = g_lc307_speed_y;
		return;
	}

	// 梯形积分：x(k)=x(k-1)+0.5*(v(k)+v(k-1))*dt
	g_lc307_pos_x += 0.5f * (g_lc307_speed_x + g_lc307_speed_x_prev) * dt_s;
	g_lc307_pos_y += 0.5f * (g_lc307_speed_y + g_lc307_speed_y_prev) * dt_s;

	// 防止长时间积分发散，先做一个安全限幅
	g_lc307_pos_x = LC307_Clamp(g_lc307_pos_x, -5.0f, 5.0f);
	g_lc307_pos_y = LC307_Clamp(g_lc307_pos_y, -5.0f, 5.0f);

	g_lc307_speed_x_prev = g_lc307_speed_x;
	g_lc307_speed_y_prev = g_lc307_speed_y;
}

// 清零累计位移和积分历史速度
void LC307_ResetPosition(void)
{
	g_lc307_pos_x = 0.0f;
	g_lc307_pos_y = 0.0f;
	g_lc307_speed_x_prev = g_lc307_speed_x;
	g_lc307_speed_y_prev = g_lc307_speed_y;
}

// 光流数据处理回调函数的弱定义
// 你的工程里如果需要更复杂的处理，可以在别的 .c 文件里重写这个函数
__weak void getOpticalFlowResult_Callback(float* buf)
{
	// 原始光流速度：buf[0]=X, buf[1]=Y
	// 这里把商家给的比例和旋转补偿逻辑封装起来，外部只读 speedX/speedY 即可
	g_lc307_speed_y = -buf[0] / 200.0f;
	g_lc307_speed_x = -buf[1] / 200.0f;

	// 使用高度和角速度补偿修正光流速度，适合后续位置积分
	g_lc307_speed_y = use_distance * (g_lc307_speed_y - LC307_Clamp(gx_filtered, -2.0f, 2.0f));
	g_lc307_speed_x = use_distance * (g_lc307_speed_x + LC307_Clamp(gy_filtered, -2.0f, 2.0f));

	// 有效数据到达后，丢失标志清零
	g_lost_pos_dev = 0;
}

// ============================
// 帧解析与中断入口
// ============================

// LC307 帧解析：校验通过后提取 flowX/flowY 并回调给上层
// 光流数据处理回调函数：在串口空闲中断中触发
static void LC307_Callback(uint16_t size)
{
	if ((size == 14U) && (serialbuffer[0] == 0xFEU)) 
    {
		// 帧头正确后，再做 XOR 校验，避免把脏数据当成有效帧
		if (serialbuffer[12] == XOR_Checksum(&serialbuffer[2], 10)) 
        {
			LC307_InitFlag = 1;
			memcpy(&opfRecv_Frame, serialbuffer, sizeof(OpticalFlowFrame_t));

			// 只把协议中的 flowX / flowY 传给上层回调，后续处理由回调决定
			{
				float raw_speed[2];
				raw_speed[0] = (float)opfRecv_Frame.flowX;
				raw_speed[1] = (float)opfRecv_Frame.flowY;
				getOpticalFlowResult_Callback(raw_speed);
			}
		}
	}
}

// 启动 LC307 连续接收（IDLE 中断 + DMA）
// 调用一次即可，后续由中断自动处理每一帧数据
void Opf_LC307_Start(void)
{
	uart1_IT_enableConfig(1);
	uart1_dma_enableConfig(1);
}

// LC307 初始化：上电后先短暂打开接收链路检测模块状态，若未持续输出则进入配置阶段
// 返回 0 表示初始化成功，非 0 表示握手/校验失败
uint8_t Opf_LC307_Init(void)
{
	uint8_t feedbackbuf[3] = {0};

	// USART1 初始化由工程公共串口驱动完成，这里直接复用
	usart_1_Init(19200);

	// LC307 使用 DMA2 接收，单独打开 DMA2 时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

	// 先短暂打开接收链路，检测模块是否已在持续输出
	uart1_IT_enableConfig(1);
	uart1_dma_enableConfig(1);
	delay_ms(200);
	uart1_IT_enableConfig(0);
	uart1_dma_enableConfig(0);

	// 若已收到合法数据帧，说明模块已初始化完成，直接返回
	if (LC307_InitFlag) 
    {
		return 0;
	} 
    else 
    {
		// 清一次 DR，避免残留数据影响后续握手
		USART_ReceiveData(USART1);
	}

	// step1：进入配置模式
	{
		uint8_t step1_initbuf[7] = {0xAA, 0xAB, 0x96, 0x26, 0xbc, 0x50, 0x5C};
		LC307_SendData(step1_initbuf, 7);
	}

	// 等待模块对 step1 的 ACK
	LC307_RecvData(feedbackbuf, 3);
	if (((feedbackbuf[0] ^ feedbackbuf[1]) != feedbackbuf[2])) 
    {
		g_lost_pos_dev = 1;
		return xor_err;
	}
	if (feedbackbuf[1] != 0x00U) 
    {
		g_lost_pos_dev = 1;
		return ack_err;
	}

	// step2 配置：按寄存器表逐项下发并校验 ACK
	// 这里是传感器的参数表，决定输出频率和内部工作模式
	for (uint16_t i = 0; i < (sizeof(tab_BF3901_60hz) / sizeof(uint8_t)); i += 2) 
    {
		uint8_t buf[5] = {0xBB, 0xDC, tab_BF3901_60hz[i], tab_BF3901_60hz[i + 1], 0};
		buf[4] = (uint8_t)(buf[1] ^ buf[2] ^ buf[3]);
		LC307_SendData(buf, 5);

		// 每条配置都要检查返回值，确保参数被模块接受
		LC307_RecvData(feedbackbuf, 3);
		if (((feedbackbuf[0] ^ feedbackbuf[1]) != feedbackbuf[2])) 
        {
			g_lost_pos_dev = 1;
			return xor_err;
		}
		if (feedbackbuf[1] != 0x00U) 
        {
			g_lost_pos_dev = 1;
			return ack_err;
		}
	}

	// step3 发送结束配置命令，进入正常输出模式
	{
		uint8_t closecfg = 0xDD;
		LC307_SendData(&closecfg, 1);
	}

	LC307_InitFlag = 1;
	return 0;
}

void USART1_IRQHandler(void)
{
	// USART1 空闲中断：表示一帧串口数据接收完成
	if (USART_GetFlagStatus(USART1, USART_FLAG_IDLE) != RESET) 
    {
		// 读 SR + DR 清除 IDLE 标志
		USART1->SR;
		USART1->DR;

		// 停止 DMA，计算本次实际接收字节数
		DMA2_Stream2->CR &= ~DMA_SxCR_EN;
		while ((DMA2_Stream2->CR & DMA_SxCR_EN) != 0U) 
        {

		}

		// 由剩余计数反推出本次帧长度，然后交给解析函数处理
		{
			uint8_t size = (uint8_t)(BUFFER_LEN - DMA2_Stream2->NDTR);
			LC307_Callback(size); //读取光流x、y速度，并做补偿
		}

		// 重新开启 DMA，准备接收下一帧数据
		start_dma_recv();
	}
}
