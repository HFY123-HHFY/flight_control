#include "NVIC_IRQ.h"

volatile uint8_t print_task_flag = 0; // printf节拍-100ms

//定时器3中断服务函数
void TIM3_IRQHandler(void)
{
    static uint16_t time_t = 0; //程序运行时间计数
    static uint8_t pif_5ms = 0; // 5ms PID节拍计数
    static uint8_t printf_100ms = 0; // 100ms printf节拍计数

    static uint8_t lc307_tick_20ms = 0; // 20ms LC307速度环节拍计数
    static uint8_t lc307_tick_40ms = 0; // 40ms LC307位置环节拍计数

	if(TIM_GetITStatus(TIM3,TIM_IT_Update) == SET) //溢出中断
	{
        pif_5ms++;
        printf_100ms++;
        lc307_tick_20ms++;
        lc307_tick_40ms++;
        time_t++;
        
/*
pid节拍-5ms
*/
        if (pif_5ms >= 5)
        {
            pif_5ms = 0;
            pid_task_flag = 1;
        }
/*
光流速度环节拍-20ms
*/
        if (lc307_tick_20ms >= 20)
        {
            lc307_tick_20ms = 0;
            lc307_speed_task_flag = 1;
        }
/*
光流位置环节拍-40ms
*/
        if (lc307_tick_40ms >= 40)
        {
            lc307_tick_40ms = 0;
            lc307_pos_task_flag = 1;
        }
/*
printf节拍-100s
*/
        if (printf_100ms >= 100)
        {
            printf_100ms = 0;
            print_task_flag = 1;
        }
 /*
程序运行时间
*/       
        if (time_t >= 1000)
        {
            time_t = 0;
            Timer_Bsp_t++;
        }	
	}
	TIM_ClearITPendingBit(TIM3,TIM_IT_Update);  //清除中断标志位
}

