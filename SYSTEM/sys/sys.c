#include "sys.h"  

/*
	正点原子
*/

// 设置单个GPIO引脚复用功能(AF0~AF15)
void GPIO_AF_Set_Reg(GPIO_TypeDef* GPIOx, uint8_t pin, uint8_t af)
{
    uint32_t idx = (uint32_t)pin >> 3U;          // 0: AFRL(0~7), 1: AFRH(8~15)
    uint32_t shift = ((uint32_t)pin & 0x7U) * 4U;

    GPIOx->AFR[idx] &= ~(0xFU << shift);
    GPIOx->AFR[idx] |= ((uint32_t)af << shift);
}

// 按位批量配置GPIO模式/输出类型/速度/上下拉
void GPIO_Set_Reg(GPIO_TypeDef* GPIOx,
                  uint16_t pinMask,
                  uint32_t mode,
                  uint32_t otype,
                  uint32_t ospeed,
                  uint32_t pupd)
{
    uint32_t pin = 0U;

    for (pin = 0U; pin < 16U; pin++)
    {
        uint32_t bit = 1U << pin;
        if ((pinMask & bit) == 0U)
        {
            continue;
        }

        // MODER: 每个引脚2位
        GPIOx->MODER &= ~(3U << (pin * 2U));
        GPIOx->MODER |= (mode << (pin * 2U));

        // 输出或复用模式下, OSPEEDR/OTYPER有效
        if ((mode == REG_GPIO_MODE_OUTPUT) || (mode == REG_GPIO_MODE_AF))
        {
            GPIOx->OSPEEDR &= ~(3U << (pin * 2U));
            GPIOx->OSPEEDR |= (ospeed << (pin * 2U));

            GPIOx->OTYPER &= ~(1U << pin);
            GPIOx->OTYPER |= (otype << pin);
        }

        GPIOx->PUPDR &= ~(3U << (pin * 2U));
        GPIOx->PUPDR |= (pupd << (pin * 2U));
    }
}

__asm void WFI_SET(void)
{
	WFI;		  
}
//关闭所有中断(但是不包括fault和NMI中断)
__asm void INTX_DISABLE(void)
{
	CPSID   I
	BX      LR	  
}
//开启所有中断
__asm void INTX_ENABLE(void)
{
	CPSIE   I
	BX      LR  
}
//设置栈顶地址
//addr:栈顶地址
__asm void MSR_MSP(u32 addr) 
{
	MSR MSP, r0 			//set Main Stack value
	BX r14
}
