#include "Pwm.h"

//Dshot300 --> 300k bit/s  PWM频率需要为 300k
// DShot300 协议位宽: 定时器周期 560 tick 时, 75%/37.5% 分别对应逻辑 1/0
#define DSHOT_BIT_1_TICKS            420U
#define DSHOT_BIT_0_TICKS            210U
#define DSHOT_FRAME_BITS             16U // DShot数据帧长度, 不包含帧尾间隔
#define DSHOT_FRAME_BUF_LEN          20U // 包含16位数据和4位帧尾间隔

// TIM1_UP 对应 DMA2 Stream5, Channel6 (STM32F407)
#define DSHOT_DMA_STREAM             DMA2_Stream5
#define DSHOT_DMA_CLEAR_MASK         (DMA_HIFCR_CFEIF5 | DMA_HIFCR_CDMEIF5 | DMA_HIFCR_CTEIF5 | DMA_HIFCR_CHTIF5 | DMA_HIFCR_CTCIF5)

static uint16_t g_dshot_cmd[DSHOT_FRAME_BUF_LEN][4] = {0};
static uint16_t g_dshot_throttle[4] = {0, 0, 0, 0};

// 将4路电机的DShot命令打包成TIM1的CCR寄存器值, 通过DMA自动更新发送
static uint16_t DShot_PacketEncode(uint16_t throttle, uint8_t telemetry)
{
    uint16_t packet;
    uint8_t crc;

    if (throttle > DSHOT_THROTTLE_MAX)
    {
        throttle = DSHOT_THROTTLE_MAX;
    }

    packet = (uint16_t)(((throttle & 0x07FFU) << 1U) | (telemetry ? 1U : 0U));
    crc = (uint8_t)((packet ^ (packet >> 4U) ^ (packet >> 8U)) & 0x0FU);
    return (uint16_t)((packet << 4U) | crc);
}

// TIM1_UP事件触发的DMA初始化, 用于自动更新TIM1的CCR寄存器发送DShot命令
static void TIM1_DShot_DMA_Init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;

    if ((DSHOT_DMA_STREAM->CR & DMA_SxCR_EN) != 0U)
    {
        DSHOT_DMA_STREAM->CR &= ~DMA_SxCR_EN;
        while ((DSHOT_DMA_STREAM->CR & DMA_SxCR_EN) != 0U)
        {
        }
    }

    DMA2->HIFCR = DSHOT_DMA_CLEAR_MASK;

    DSHOT_DMA_STREAM->CR = 0U;
    DSHOT_DMA_STREAM->CR = (6U << 25U) | // CHSEL=6 -> TIM1_UP
                           DMA_SxCR_DIR_0 | // Memory-to-peripheral
                           DMA_SxCR_MINC |
                           DMA_SxCR_PSIZE_0 | // 16-bit peripheral
                           DMA_SxCR_MSIZE_0 | // 16-bit memory
                           DMA_SxCR_PL_1; // high priority

    DSHOT_DMA_STREAM->NDTR = DSHOT_FRAME_BUF_LEN * 4U;
    DSHOT_DMA_STREAM->PAR = (uint32_t)&(TIM1->DMAR);
    DSHOT_DMA_STREAM->M0AR = (uint32_t)g_dshot_cmd;
    DSHOT_DMA_STREAM->FCR = 0U;

    // Update事件触发DMA burst, 每次更新CCR1~CCR4四个通道
    TIM_DMACmd(TIM1, TIM_DMA_Update, ENABLE);
    TIM_DMAConfig(TIM1, TIM_DMABase_CCR1, TIM_DMABurstLength_4Transfers);
}

// 直接发送4路电机的DShot命令, 通过DMA自动更新TIM1的CCR寄存器
static void TIM1_DShot_SendFrame(uint16_t m1, uint16_t m2, uint16_t m3, uint16_t m4)
{
    uint16_t packet1 = DShot_PacketEncode(m1, 0U);
    uint16_t packet2 = DShot_PacketEncode(m2, 0U);
    uint16_t packet3 = DShot_PacketEncode(m3, 0U);
    uint16_t packet4 = DShot_PacketEncode(m4, 0U);
    uint8_t i;

    for (i = 0U; i < DSHOT_FRAME_BITS; i++)
    {
        g_dshot_cmd[i][0] = ((packet1 >> (15U - i)) & 0x01U) ? DSHOT_BIT_1_TICKS : DSHOT_BIT_0_TICKS;
        g_dshot_cmd[i][1] = ((packet2 >> (15U - i)) & 0x01U) ? DSHOT_BIT_1_TICKS : DSHOT_BIT_0_TICKS;
        g_dshot_cmd[i][2] = ((packet3 >> (15U - i)) & 0x01U) ? DSHOT_BIT_1_TICKS : DSHOT_BIT_0_TICKS;
        g_dshot_cmd[i][3] = ((packet4 >> (15U - i)) & 0x01U) ? DSHOT_BIT_1_TICKS : DSHOT_BIT_0_TICKS;
    }

    // 留出帧尾间隔, 让电调可靠分帧
    for (i = DSHOT_FRAME_BITS; i < DSHOT_FRAME_BUF_LEN; i++)
    {
        g_dshot_cmd[i][0] = 0U;
        g_dshot_cmd[i][1] = 0U;
        g_dshot_cmd[i][2] = 0U;
        g_dshot_cmd[i][3] = 0U;
    }

    if ((DSHOT_DMA_STREAM->CR & DMA_SxCR_EN) != 0U)
    {
        DSHOT_DMA_STREAM->CR &= ~DMA_SxCR_EN;
        while ((DSHOT_DMA_STREAM->CR & DMA_SxCR_EN) != 0U)
        {
        }
    }

    DMA2->HIFCR = DSHOT_DMA_CLEAR_MASK;
    DSHOT_DMA_STREAM->M0AR = (uint32_t)g_dshot_cmd;
    DSHOT_DMA_STREAM->NDTR = DSHOT_FRAME_BUF_LEN * 4U;
    DSHOT_DMA_STREAM->CR |= DMA_SxCR_EN;

    while ((DMA2->HISR & DMA_HISR_TCIF5) == 0U)
    {
    }

    DSHOT_DMA_STREAM->CR &= ~DMA_SxCR_EN;
    while ((DSHOT_DMA_STREAM->CR & DMA_SxCR_EN) != 0U)
    {
    }
    DMA2->HIFCR = DSHOT_DMA_CLEAR_MASK;

    // 帧完成后拉低输出, 防止保持上一次占空比
    TIM1->CCR1 = 0U;
    TIM1->CCR2 = 0U;
    TIM1->CCR3 = 0U;
    TIM1->CCR4 = 0U;
    TIM1->EGR |= TIM_EGR_UG;
}

// IM1四通道 DShot300 PWM 初始化
void TIM1_PWM_Init(u32 arr, u32 psc)
{
    // 1) 开时钟: TIM1 在 APB2, GPIOE 在 AHB1
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;

    // 2) 配置 PE9/PE11/PE13/PE14 为 TIM1_CH1/2/3/4 复用输出
    //    DShot300 需要稳定的高速方波, 因此使用复用功能、推挽输出、上拉和高速驱动
    GPIO_Set_Reg(GPIOE,
                 (uint16_t)((1U << 9U) | (1U << 11U) | (1U << 13U) | (1U << 14U)),
                 REG_GPIO_MODE_AF,
                 REG_GPIO_OTYPE_PP,
                 REG_GPIO_SPEED_HIGH,
                 REG_GPIO_PUPD_UP);

    GPIO_AF_Set_Reg(GPIOE, 9U, 1U);
    GPIO_AF_Set_Reg(GPIOE, 11U, 1U);
    GPIO_AF_Set_Reg(GPIOE, 13U, 1U);
    GPIO_AF_Set_Reg(GPIOE, 14U, 1U);

    // 3) 配置 TIM1 基准频率:
    //    f_pwm = 168MHz / ((PSC+1) * (ARR+1))
    //    传入 arr=560, psc=1 时, 实际输出约为 168MHz / (1 * 560) = 300kHz
    TIM1->CR1 &= ~TIM_CR1_CEN;
    TIM1->PSC = psc - 1U;
    TIM1->ARR = arr - 1U;

    // 4) PWM 模式 1 + CCR 预装载(4 通道)
    //    OCxM = 110 表示 PWM1, OCxPE = 1 使能比较寄存器预装载
    TIM1->CCMR1 &= ~(TIM_CCMR1_CC1S | TIM_CCMR1_OC1M | TIM_CCMR1_OC1PE |
                     TIM_CCMR1_CC2S | TIM_CCMR1_OC2M | TIM_CCMR1_OC2PE);
    TIM1->CCMR1 |=  (TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1PE |
                     TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2PE);

    TIM1->CCMR2 &= ~(TIM_CCMR2_CC3S | TIM_CCMR2_OC3M | TIM_CCMR2_OC3PE |
                     TIM_CCMR2_CC4S | TIM_CCMR2_OC4M | TIM_CCMR2_OC4PE);
    TIM1->CCMR2 |=  (TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3PE |
                     TIM_CCMR2_OC4M_1 | TIM_CCMR2_OC4M_2 | TIM_CCMR2_OC4PE);

    // 5) 打开 4 个输出通道, 极性配置为高电平有效
    TIM1->CCER &= ~(TIM_CCER_CC1P | TIM_CCER_CC2P | TIM_CCER_CC3P | TIM_CCER_CC4P);
    TIM1->CCER |= (TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E | TIM_CCER_CC4E);

    // 6) 初始占空比为 0, 避免上电瞬间误触发电调
    TIM1->CCR1 = 0U;
    TIM1->CCR2 = 0U;
    TIM1->CCR3 = 0U;
    TIM1->CCR4 = 0U;

    // 7) 高级定时器 TIM1 需要 MOE 才会真正从引脚输出
    TIM1->BDTR |= TIM_BDTR_MOE;

    // 8) 使能 ARR 预装载并触发一次更新事件, 将预装载值同步到影子寄存器
    TIM1->CR1 |= TIM_CR1_ARPE;
    TIM1->EGR |= TIM_EGR_UG;

    // 9) 启动计数器, 此时 TIM1 输出 DShot300 所需的 300kHz PWM 基波
    TIM1->CR1 |= TIM_CR1_CEN;

    // 10) 初始化DMA, 后续通过DMA burst把DShot位流装载到CCR1~CCR4
    TIM1_DShot_DMA_Init();
}

// 将输入的油门值限制在 DShot 协议允许的范围内, 并转换为整数
void TIM1_DShot_Write(uint16_t m1, uint16_t m2, uint16_t m3, uint16_t m4)
{
    g_dshot_throttle[0] = m3; //3
    g_dshot_throttle[1] = m1; //1
    g_dshot_throttle[2] = m2; //2
    g_dshot_throttle[3] = m4; //4

    TIM1_DShot_SendFrame(g_dshot_throttle[0],
                         g_dshot_throttle[1],
                         g_dshot_throttle[2],
                         g_dshot_throttle[3]);
}

void MOS3_Control(uint16_t duty)
{
    g_dshot_throttle[2] = duty;
    TIM1_DShot_SendFrame(g_dshot_throttle[0],
                         g_dshot_throttle[1],
                         g_dshot_throttle[2],
                         g_dshot_throttle[3]);
}

void MOS1_Control(uint16_t duty)
{
    g_dshot_throttle[0] = duty;
    TIM1_DShot_SendFrame(g_dshot_throttle[0],
                         g_dshot_throttle[1],
                         g_dshot_throttle[2],
                         g_dshot_throttle[3]);
}

void MOS2_Control(uint16_t duty)
{
    g_dshot_throttle[1] = duty;
    TIM1_DShot_SendFrame(g_dshot_throttle[0],
                         g_dshot_throttle[1],
                         g_dshot_throttle[2],
                         g_dshot_throttle[3]);
}

void MOS4_Control(uint16_t duty)
{
    g_dshot_throttle[3] = duty;
    TIM1_DShot_SendFrame(g_dshot_throttle[0],
                         g_dshot_throttle[1],
                         g_dshot_throttle[2],
                         g_dshot_throttle[3]);
}
