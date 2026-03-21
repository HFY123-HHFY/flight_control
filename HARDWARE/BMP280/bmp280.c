#include "bmp280.h"

float alt = 0; // 高度

/*bmp280 气压和温度过采样 工作模式*/
#define BMP280_PRESSURE_OSR			(BMP280_OVERSAMP_8X)
#define BMP280_TEMPERATURE_OSR		(BMP280_OVERSAMP_16X)
#define BMP280_MODE					(BMP280_PRESSURE_OSR<<2|BMP280_TEMPERATURE_OSR<<5|BMP280_NORMAL_MODE)

typedef struct 
{
    u16 dig_T1;                                                                /* calibration T1 data */
    s16 dig_T2;                                                                /* calibration T2 data */
    s16 dig_T3;                                                                /* calibration T3 data */
    u16 dig_P1;                                                                /* calibration P1 data */
    s16 dig_P2;                                                                /* calibration P2 data */
    s16 dig_P3;                                                                /* calibration P3 data */
    s16 dig_P4;                                                                /* calibration P4 data */
    s16 dig_P5;                                                                /* calibration P5 data */
    s16 dig_P6;                                                                /* calibration P6 data */
    s16 dig_P7;                                                                /* calibration P7 data */
    s16 dig_P8;                                                                /* calibration P8 data */
    s16 dig_P9;                                                                /* calibration P9 data */
    s32 t_fine;                                                                /* calibration t_fine data */
} bmp280Calib;

bmp280Calib  bmp280Cal;

static u8 bmp280ID = 0;
static bool isInit=false;
static s32 bmp280RawPressure=0;
static s32 bmp280RawTemperature=0;

static void bmp280GetPressure(void);
static void presssureFilter(float* in,float* out);
static float bmp280PressureToAltitude(float* pressure/*, float* groundPressure, float* groundTemp*/);

//从指定地址读出一个数据
//ReadAddr:开始读数的地址  
//返回值  :读到的数据
u8 iicDevReadByte(u8 devaddr,u8 addr)
{				  
	u8 temp=0;		  	    																 
	MyI2C_Start();  
	MyI2C_SendByte(devaddr);                                                    //发送器件写命令 	   
	MyI2C_Wait_Ack(); 
	MyI2C_SendByte(addr);                                                       //发送低地址
	MyI2C_Wait_Ack();	

	MyI2C_Start();  	 	   
	MyI2C_SendByte(devaddr|1);                                                  //发送器件读命令			   
	MyI2C_Wait_Ack();	 
	temp=MyI2C_ReceiveByte(0);			   
	MyI2C_Stop();                                                                //产生一个停止条件	    
	return temp;
}

//连续读多个字节
//addr:起始地址
//rbuf:读数据缓存
//len:数据长度
void iicDevRead(u8 devaddr,u8 addr,u8 len,u8 *rbuf)
{
	int i=0;
	MyI2C_Start();  
	MyI2C_SendByte(devaddr);  
	MyI2C_Wait_Ack();	
	MyI2C_SendByte(addr);                                                       //地址自增  
	MyI2C_Wait_Ack();	

	MyI2C_Start();  	
	MyI2C_SendByte(devaddr|1);  	
	MyI2C_Wait_Ack();	
	for(i=0; i<len; i++)
	{
		if(i==len-1)
		{
			rbuf[i]=MyI2C_ReceiveByte(0);                                          //最后一个字节不应答
		}
		else
			rbuf[i]=MyI2C_ReceiveByte(1);
	}
	MyI2C_Stop( );	
}

//从指定地址写入一个数据
//WriteAddr :写入数据的目的地址    
//DataToWrite:要写入的数据
void iicDevWriteByte(u8 devaddr,u8 addr,u8 data)
{				   	  	    																 
	MyI2C_Start();  
	MyI2C_SendByte(devaddr);                                                    //发送器件写命令 	 
	MyI2C_Wait_Ack();	   
	MyI2C_SendByte(addr);                                                       //发送低地址
	MyI2C_Wait_Ack(); 	 										  		   
	MyI2C_SendByte(data);                                                       //发送字节							   
	MyI2C_Wait_Ack();  		    	   
	MyI2C_Stop();		                                                           //产生一个停止条件 	 
}

//连续写多个字节
//addr:起始地址
//wbuf:写数据缓存
//len:数据的长度
void iicDevWrite(u8 devaddr,u8 addr,u8 len,u8 *wbuf)
{
	int i=0;
	MyI2C_Start();  
	MyI2C_SendByte(devaddr);  	
	MyI2C_Wait_Ack();	
	MyI2C_SendByte(addr);  //地址自增
	MyI2C_Wait_Ack();	
	for(i=0; i<len; i++)
	{
		MyI2C_SendByte(wbuf[i]);  
		MyI2C_Wait_Ack();		
	}
	MyI2C_Stop( );	
}

bool BMP280Init(void)
{	
	if (isInit)
		return true;

	bmp280ID = iicDevReadByte(BMP280_ADDR,BMP280_CHIP_ID);	                   /* 读取bmp280 ID*/
	
	if(bmp280ID == BMP280_DEFAULT_CHIP_ID)
		printf("BMP280 ID: 0x%X\n",bmp280ID); // 0x58
    else
        return false;

    /* 读取校准数据 */
	iicDevRead(BMP280_ADDR,BMP280_TEMPERATURE_CALIB_DIG_T1_LSB_REG,24,(u8 *)&bmp280Cal);	
	iicDevWriteByte(BMP280_ADDR,BMP280_CTRL_MEAS_REG,BMP280_MODE);
	iicDevWriteByte(BMP280_ADDR,BMP280_CONFIG_REG,(0 << 5) | (4 << 2) | 0);		               /*配置IIR滤波*/
	
    isInit=true;
    return true;
}

static void bmp280GetPressure(void)
{
    u8 data[BMP280_DATA_FRAME_SIZE];

    // read data from sensor
    iicDevRead(BMP280_ADDR,BMP280_PRESSURE_MSB_REG,BMP280_DATA_FRAME_SIZE,data);
    bmp280RawPressure=(s32)((((uint32_t)(data[0]))<<12)|(((uint32_t)(data[1]))<<4)|((uint32_t)data[2]>>4));
    bmp280RawTemperature=(s32)((((uint32_t)(data[3]))<<12)|(((uint32_t)(data[4]))<<4)|((uint32_t)data[5]>>4));
}

// Returns temperature in DegC, resolution is 0.01 DegC. Output value of "5123" equals 51.23 DegC
// t_fine carries fine temperature as global value
static s32 bmp280CompensateT(s32 adcT)
{
    s32 var1,var2,T;

    var1=((((adcT>>3)-((s32)bmp280Cal.dig_T1<<1)))*((s32)bmp280Cal.dig_T2))>>11;
    var2=(((((adcT>>4)-((s32)bmp280Cal.dig_T1))*((adcT>>4)-((s32)bmp280Cal.dig_T1)))>>12)*((s32)bmp280Cal.dig_T3))>>14;
    bmp280Cal.t_fine=var1+var2;
	
    T=(bmp280Cal.t_fine*5+128)>>8;

    return T;
}

// 以 Q24.8 格式（24 个整数位和 8 个小数位）将压力以 Pa 为单位返回为无符号 32 位整数。
// “24674867”的输出值表示 24674867/256 = 96386.2 Pa = 963.862 hPa
static uint32_t bmp280CompensateP(s32 adcP)
{
    int64_t var1,var2,p;
    var1=((int64_t)bmp280Cal.t_fine)-128000;
    var2=var1*var1*(int64_t)bmp280Cal.dig_P6;
    var2=var2+((var1*(int64_t)bmp280Cal.dig_P5)<<17);
    var2=var2+(((int64_t)bmp280Cal.dig_P4)<<35);
    var1=((var1*var1*(int64_t)bmp280Cal.dig_P3)>>8)+((var1*(int64_t)bmp280Cal.dig_P2)<<12);
    var1=(((((int64_t)1)<<47)+var1))*((int64_t)bmp280Cal.dig_P1)>>33;
    if (var1==0)
        return 0;
    p=1048576-adcP;
    p=(((p<<31)-var2)*3125)/var1;
    var1=(((int64_t)bmp280Cal.dig_P9)*(p>>13)*(p>>13))>>25;
    var2=(((int64_t)bmp280Cal.dig_P8)*p)>>19;
    p=((p+var1+var2)>>8)+(((int64_t)bmp280Cal.dig_P7)<<4);
    return(uint32_t)p;
}

#define FILTER_NUM	5
#define FILTER_A	0.1f

/*限幅平均滤波法*/
static void presssureFilter(float* in,float* out)
{	
	static u8 i=0;
	static float filter_buf[FILTER_NUM]={0.0};
	double filter_sum=0.0;
	u8 cnt=0;	
	float deta;

	if(filter_buf[i]==0.0f)
	{
		filter_buf[i]=*in;
		*out=*in;
		if(++i>=FILTER_NUM)	
			i=0;
	} 
	else 
	{
		if(i)
			deta=*in-filter_buf[i-1];
		else 
			deta=*in-filter_buf[FILTER_NUM-1];
		
		if(fabs(deta)<FILTER_A)
		{
			filter_buf[i]=*in;
			if(++i>=FILTER_NUM)	
				i=0;
		}
		for(cnt=0;cnt<FILTER_NUM;cnt++)
		{
			filter_sum+=filter_buf[cnt];
		}
		*out=filter_sum /FILTER_NUM;
	}
}

void BMP280GetData(float* pressure,float* temperature,float* asl)
{
	static float t;
	static float p;
	
	bmp280GetPressure();

	t=bmp280CompensateT(bmp280RawTemperature)/100.0;		
	p=bmp280CompensateP(bmp280RawPressure)/25600.0;		

	presssureFilter(&p,pressure);
	*temperature=(float)t;                                                   /*单位度*/
	*pressure=(float)p ;	                                                   /*单位hPa*/	
	
	*asl=bmp280PressureToAltitude(pressure);	                               /*转换成海拔*/	
}

#define CONST_PF 0.1902630958	                                               //(1/5.25588f) Pressure factor
#define FIX_TEMP 25				                                               // Fixed Temperature. ASL is a function of pressure and temperature, but as the temperature changes so much (blow a little towards the flie and watch it drop 5 degrees) it corrupts the ASL estimates.
								                                               // TLDR: Adjusting for temp changes does more harm than good.
/*
 * Converts pressure to altitude above sea level (ASL) in meters
*/
static float bmp280PressureToAltitude(float* pressure/*, float* groundPressure, float* groundTemp*/)
{
    if (*pressure>0)
    { //1015.7f
        return((pow((1015.7f/ *pressure),CONST_PF)-1.0f)*(FIX_TEMP+273.15f))/0.0065f;
    }
    else
    {
        return 0;
    }
}

float BMP_Data(void)
{
		float bmp280_press = 0.0f,bmp280 = 0.0f;				//气压,海拔高度
		BMP280GetData(&bmp280_press,&bmp280,&bmp280); // 获取气压和海拔高度
		return bmp280;
}
