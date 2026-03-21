#include "NVIC_IRQ.h"

//定时器3中断服务函数
void TIM3_IRQHandler(void)
{
    static uint16_t time_t = 0, Count0 = 0;

	if(TIM_GetITStatus(TIM3,TIM_IT_Update) == SET) //溢出中断
	{
        Count0++;
        time_t++;

        if (Count0 >= 5)
        {
            Count0 = 0;
            pid_task_flag = 1;
        }
        
        if (time_t >= 1000)
        {
            time_t = 0;
            Timer_Bsp_t++;
        }	
	}
	TIM_ClearITPendingBit(TIM3,TIM_IT_Update);  //清除中断标志位
}

