#include "adc.h"

uint16_t ADValue[4] = {0,0,0,0};			//定义AD值变量
float Voltage[4] = {0,0,0,0};				//定义电压变量
uint32_t ADC_TimeoutCount = 0;
uint32_t ADC_ResErrorCount = 0;

//初始化ADC															   
void Adc_Init(void)
{    
	GPIO_InitTypeDef  GPIO_InitStructure;
	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	ADC_InitTypeDef       ADC_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);//使能GPIOA时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE); //使能ADC1时钟

	//先初始化ADC1通道5 IO口
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;//模拟输入
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;//不带上下拉
	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化  

	RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1,ENABLE);	  //ADC1复位
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1,DISABLE);	//复位结束	 

	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;//独立模式
	ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;//两个采样阶段之间的延迟5个时钟
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled; //DMA失能
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;//预分频4分频。ADCCLK=PCLK2/4=84/4=21Mhz,ADC时钟最好不要超过36Mhz 
	ADC_CommonInit(&ADC_CommonInitStructure);//初始化

	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;//12位模式
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;//非扫描模式	
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;//关闭连续转换
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;//禁止触发检测，使用软件触发
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;//边沿为None时该项不生效，这里显式赋值避免未定义状态
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;//右对齐	
	ADC_InitStructure.ADC_NbrOfConversion = 1;//1个转换在规则序列中 也就是只转换规则序列1 
	ADC_Init(ADC1, &ADC_InitStructure);//ADC初始化
	ADC_EOCOnEachRegularChannelCmd(ADC1, ENABLE); // 每次规则转换完成都置位EOC

	ADC_Cmd(ADC1, ENABLE);//开启AD转换器	
	ADC_ClearFlag(ADC1, ADC_FLAG_EOC | ADC_FLAG_OVR);
	delay_us(10); // 给ADC稳定一点时间，避免上电后首转换异常
}

//获得ADC值
//ch: @ref ADC_channels 
//通道值 0~16取值范围为：ADC_Channel_0~ADC_Channel_16
//返回值:转换结果
u16 Get_Adc(u8 ch)   
{
	u32 timeout = 1000000;

	if ((ADC1->CR1 & ADC_CR1_RES) != 0)
	{
		ADC_ResErrorCount++;
		ADC1->CR1 &= ~ADC_CR1_RES; // 强制回到12位分辨率
	}

	//设置指定ADC的规则组通道，一个序列，采样时间
	ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_480Cycles );	//ADC1,ADC通道,480个周期,提高采样时间可以提高精确度			    
	ADC_ClearFlag(ADC1, ADC_FLAG_EOC | ADC_FLAG_OVR);

	ADC_SoftwareStartConv(ADC1);		//使能指定的ADC1的软件转换启动功能	

	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ))
	{
		if (timeout-- == 0)
		{
			ADC_TimeoutCount++;
			return 0xFFFF;
		}
	}

	return (ADC_GetConversionValue(ADC1) & 0x0FFF);	//返回12位ADC结果
}

//获取通道ch的转换值，取times次,然后平均 
//ch:通道编号
//times:获取次数
//返回值:通道ch的times次转换结果平均值
//Get_Adc_Average(ADC_Channel_5,20);//获取通道5的转换值，20次取平均
u16 Get_Adc_Average(u8 ch,u8 times)
{
	u32 temp_val=0;
	u8 t;
	for(t=0;t<times;t++)
	{
		temp_val += Get_Adc(ch);
		delay_us(20);
	}
	return temp_val/times;
}

void ADC_Get(void)
{
	Get_Adc(ADC_Channel_0);
	ADValue[0] = Get_Adc_Average(ADC_Channel_0, 8);
	Get_Adc(ADC_Channel_1);
	ADValue[1] = Get_Adc_Average(ADC_Channel_1, 8);
	Get_Adc(ADC_Channel_2);
	ADValue[2] = Get_Adc_Average(ADC_Channel_2, 8);
	Get_Adc(ADC_Channel_3);
	ADValue[3] = Get_Adc_Average(ADC_Channel_3, 8);

	// Voltage[0] = (float)ADValue[0]*(3.3/4096);          //获取计算后的带小数的实际电压值，比如3.1111
	// Voltage[1] = (float)ADValue[1]*(3.3/4096);          //获取计算后的带小数的实际电压值，比如3.1111
	// Voltage[2] = (float)ADValue[2]*(3.3/4096);          //获取计算后的带小数的实际电压值，比如3.1111
	// Voltage[3] = (float)ADValue[3]*(3.3/4096);          //获取计算后的带小数的实际电压值，比如3.1111
}
