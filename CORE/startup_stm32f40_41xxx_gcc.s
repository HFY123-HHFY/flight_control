    .syntax unified
    .cpu cortex-m4
    .fpu fpv4-sp-d16
    .thumb

    .global g_pfnVectors
    .global Default_Handler
    .global Reset_Handler

    .extern SystemInit
    .extern main
    .extern __libc_init_array

    .section .isr_vector,"a",%progbits
    .type g_pfnVectors, %object
g_pfnVectors:
    .word _estack
    .word Reset_Handler
    .word NMI_Handler
    .word HardFault_Handler
    .word MemManage_Handler
    .word BusFault_Handler
    .word UsageFault_Handler
    .word 0
    .word 0
    .word 0
    .word 0
    .word SVC_Handler
    .word DebugMon_Handler
    .word 0
    .word PendSV_Handler
    .word SysTick_Handler
    .word WWDG_IRQHandler
    .word PVD_IRQHandler
    .word TAMP_STAMP_IRQHandler
    .word RTC_WKUP_IRQHandler
    .word FLASH_IRQHandler
    .word RCC_IRQHandler
    .word EXTI0_IRQHandler
    .word EXTI1_IRQHandler
    .word EXTI2_IRQHandler
    .word EXTI3_IRQHandler
    .word EXTI4_IRQHandler
    .word DMA1_Stream0_IRQHandler
    .word DMA1_Stream1_IRQHandler
    .word DMA1_Stream2_IRQHandler
    .word DMA1_Stream3_IRQHandler
    .word DMA1_Stream4_IRQHandler
    .word DMA1_Stream5_IRQHandler
    .word DMA1_Stream6_IRQHandler
    .word ADC_IRQHandler
    .word CAN1_TX_IRQHandler
    .word CAN1_RX0_IRQHandler
    .word CAN1_RX1_IRQHandler
    .word CAN1_SCE_IRQHandler
    .word EXTI9_5_IRQHandler
    .word TIM1_BRK_TIM9_IRQHandler
    .word TIM1_UP_TIM10_IRQHandler
    .word TIM1_TRG_COM_TIM11_IRQHandler
    .word TIM1_CC_IRQHandler
    .word TIM2_IRQHandler
    .word TIM3_IRQHandler
    .word TIM4_IRQHandler
    .word I2C1_EV_IRQHandler
    .word I2C1_ER_IRQHandler
    .word I2C2_EV_IRQHandler
    .word I2C2_ER_IRQHandler
    .word SPI1_IRQHandler
    .word SPI2_IRQHandler
    .word USART1_IRQHandler
    .word USART2_IRQHandler
    .word USART3_IRQHandler
    .word EXTI15_10_IRQHandler
    .word RTC_Alarm_IRQHandler
    .word OTG_FS_WKUP_IRQHandler
    .word TIM8_BRK_TIM12_IRQHandler
    .word TIM8_UP_TIM13_IRQHandler
    .word TIM8_TRG_COM_TIM14_IRQHandler
    .word TIM8_CC_IRQHandler
    .word DMA1_Stream7_IRQHandler
    .word FSMC_IRQHandler
    .word SDIO_IRQHandler
    .word TIM5_IRQHandler
    .word SPI3_IRQHandler
    .word UART4_IRQHandler
    .word UART5_IRQHandler
    .word TIM6_DAC_IRQHandler
    .word TIM7_IRQHandler
    .word DMA2_Stream0_IRQHandler
    .word DMA2_Stream1_IRQHandler
    .word DMA2_Stream2_IRQHandler
    .word DMA2_Stream3_IRQHandler
    .word DMA2_Stream4_IRQHandler
    .word ETH_IRQHandler
    .word ETH_WKUP_IRQHandler
    .word CAN2_TX_IRQHandler
    .word CAN2_RX0_IRQHandler
    .word CAN2_RX1_IRQHandler
    .word CAN2_SCE_IRQHandler
    .word OTG_FS_IRQHandler
    .word DMA2_Stream5_IRQHandler
    .word DMA2_Stream6_IRQHandler
    .word DMA2_Stream7_IRQHandler
    .word USART6_IRQHandler
    .word I2C3_EV_IRQHandler
    .word I2C3_ER_IRQHandler
    .word OTG_HS_EP1_OUT_IRQHandler
    .word OTG_HS_EP1_IN_IRQHandler
    .word OTG_HS_WKUP_IRQHandler
    .word OTG_HS_IRQHandler
    .word DCMI_IRQHandler
    .word CRYP_IRQHandler
    .word HASH_RNG_IRQHandler
    .word FPU_IRQHandler
    .size g_pfnVectors, . - g_pfnVectors

    .section .text.Reset_Handler,"ax",%progbits
    .type Reset_Handler, %function
Reset_Handler:
    ldr sp, =_estack

    ldr r0, =_sidata
    ldr r1, =_sdata
    ldr r2, =_edata
1:
    cmp r1, r2
    bcc 2f
    b 3f
2:
    ldr r3, [r0], #4
    str r3, [r1], #4
    b 1b
3:
    ldr r0, =_sbss
    ldr r1, =_ebss
    movs r2, #0
4:
    cmp r0, r1
    bcc 5f
    b 6f
5:
    str r2, [r0], #4
    b 4b
6:
    bl SystemInit
    bl __libc_init_array
    bl main
7:
    b 7b
    .size Reset_Handler, . - Reset_Handler

    .thumb_set NMI_Handler, Default_Handler
    .thumb_set HardFault_Handler, Default_Handler
    .thumb_set MemManage_Handler, Default_Handler
    .thumb_set BusFault_Handler, Default_Handler
    .thumb_set UsageFault_Handler, Default_Handler
    .thumb_set SVC_Handler, Default_Handler
    .thumb_set DebugMon_Handler, Default_Handler
    .thumb_set PendSV_Handler, Default_Handler
    .thumb_set SysTick_Handler, Default_Handler
    .thumb_set WWDG_IRQHandler, Default_Handler
    .thumb_set PVD_IRQHandler, Default_Handler
    .thumb_set TAMP_STAMP_IRQHandler, Default_Handler
    .thumb_set RTC_WKUP_IRQHandler, Default_Handler
    .thumb_set FLASH_IRQHandler, Default_Handler
    .thumb_set RCC_IRQHandler, Default_Handler
    .thumb_set EXTI0_IRQHandler, Default_Handler
    .thumb_set EXTI1_IRQHandler, Default_Handler
    .thumb_set EXTI2_IRQHandler, Default_Handler
    .thumb_set EXTI3_IRQHandler, Default_Handler
    .thumb_set EXTI4_IRQHandler, Default_Handler
    .thumb_set DMA1_Stream0_IRQHandler, Default_Handler
    .thumb_set DMA1_Stream1_IRQHandler, Default_Handler
    .thumb_set DMA1_Stream2_IRQHandler, Default_Handler
    .thumb_set DMA1_Stream3_IRQHandler, Default_Handler
    .thumb_set DMA1_Stream4_IRQHandler, Default_Handler
    .thumb_set DMA1_Stream5_IRQHandler, Default_Handler
    .thumb_set DMA1_Stream6_IRQHandler, Default_Handler
    .thumb_set ADC_IRQHandler, Default_Handler
    .thumb_set CAN1_TX_IRQHandler, Default_Handler
    .thumb_set CAN1_RX0_IRQHandler, Default_Handler
    .thumb_set CAN1_RX1_IRQHandler, Default_Handler
    .thumb_set CAN1_SCE_IRQHandler, Default_Handler
    .thumb_set EXTI9_5_IRQHandler, Default_Handler
    .thumb_set TIM1_BRK_TIM9_IRQHandler, Default_Handler
    .thumb_set TIM1_UP_TIM10_IRQHandler, Default_Handler
    .thumb_set TIM1_TRG_COM_TIM11_IRQHandler, Default_Handler
    .thumb_set TIM1_CC_IRQHandler, Default_Handler
    .thumb_set TIM2_IRQHandler, Default_Handler
    .thumb_set TIM3_IRQHandler, Default_Handler
    .thumb_set TIM4_IRQHandler, Default_Handler
    .thumb_set I2C1_EV_IRQHandler, Default_Handler
    .thumb_set I2C1_ER_IRQHandler, Default_Handler
    .thumb_set I2C2_EV_IRQHandler, Default_Handler
    .thumb_set I2C2_ER_IRQHandler, Default_Handler
    .thumb_set SPI1_IRQHandler, Default_Handler
    .thumb_set SPI2_IRQHandler, Default_Handler
    .thumb_set USART1_IRQHandler, Default_Handler
    .thumb_set USART2_IRQHandler, Default_Handler
    .thumb_set USART3_IRQHandler, Default_Handler
    .thumb_set EXTI15_10_IRQHandler, Default_Handler
    .thumb_set RTC_Alarm_IRQHandler, Default_Handler
    .thumb_set OTG_FS_WKUP_IRQHandler, Default_Handler
    .thumb_set TIM8_BRK_TIM12_IRQHandler, Default_Handler
    .thumb_set TIM8_UP_TIM13_IRQHandler, Default_Handler
    .thumb_set TIM8_TRG_COM_TIM14_IRQHandler, Default_Handler
    .thumb_set TIM8_CC_IRQHandler, Default_Handler
    .thumb_set DMA1_Stream7_IRQHandler, Default_Handler
    .thumb_set FSMC_IRQHandler, Default_Handler
    .thumb_set SDIO_IRQHandler, Default_Handler
    .thumb_set TIM5_IRQHandler, Default_Handler
    .thumb_set SPI3_IRQHandler, Default_Handler
    .thumb_set UART4_IRQHandler, Default_Handler
    .thumb_set UART5_IRQHandler, Default_Handler
    .thumb_set TIM6_DAC_IRQHandler, Default_Handler
    .thumb_set TIM7_IRQHandler, Default_Handler
    .thumb_set DMA2_Stream0_IRQHandler, Default_Handler
    .thumb_set DMA2_Stream1_IRQHandler, Default_Handler
    .thumb_set DMA2_Stream2_IRQHandler, Default_Handler
    .thumb_set DMA2_Stream3_IRQHandler, Default_Handler
    .thumb_set DMA2_Stream4_IRQHandler, Default_Handler
    .thumb_set ETH_IRQHandler, Default_Handler
    .thumb_set ETH_WKUP_IRQHandler, Default_Handler
    .thumb_set CAN2_TX_IRQHandler, Default_Handler
    .thumb_set CAN2_RX0_IRQHandler, Default_Handler
    .thumb_set CAN2_RX1_IRQHandler, Default_Handler
    .thumb_set CAN2_SCE_IRQHandler, Default_Handler
    .thumb_set OTG_FS_IRQHandler, Default_Handler
    .thumb_set DMA2_Stream5_IRQHandler, Default_Handler
    .thumb_set DMA2_Stream6_IRQHandler, Default_Handler
    .thumb_set DMA2_Stream7_IRQHandler, Default_Handler
    .thumb_set USART6_IRQHandler, Default_Handler
    .thumb_set I2C3_EV_IRQHandler, Default_Handler
    .thumb_set I2C3_ER_IRQHandler, Default_Handler
    .thumb_set OTG_HS_EP1_OUT_IRQHandler, Default_Handler
    .thumb_set OTG_HS_EP1_IN_IRQHandler, Default_Handler
    .thumb_set OTG_HS_WKUP_IRQHandler, Default_Handler
    .thumb_set OTG_HS_IRQHandler, Default_Handler
    .thumb_set DCMI_IRQHandler, Default_Handler
    .thumb_set CRYP_IRQHandler, Default_Handler
    .thumb_set HASH_RNG_IRQHandler, Default_Handler
    .thumb_set FPU_IRQHandler, Default_Handler

    .section .text.Default_Handler,"ax",%progbits
    .type Default_Handler, %function
Default_Handler:
    b .
    .size Default_Handler, . - Default_Handler

    .section .text._init,"ax",%progbits
    .global _init
    .type _init, %function
_init:
    bx lr
    .size _init, . - _init
