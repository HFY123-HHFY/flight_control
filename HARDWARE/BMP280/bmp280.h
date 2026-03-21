#ifndef __BMP280_H
#define __BMP280_H
#include "sys.h"
#include "stdbool.h"
#include <math.h>
#include "My_I2C.h"
#include "usart.h"

#define BMP280_ADDR						(0xEC)
#define BMP280_DEFAULT_CHIP_ID			(0x58)

#define BMP280_CHIP_ID					(0xD0)                                 /* Chip ID Register */
#define BMP280_RST_REG					(0xE0)                                 /* Softreset Register */
#define BMP280_STAT_REG					(0xF3)                                 /* Status Register */
#define BMP280_CTRL_MEAS_REG			(0xF4)                                 /* Ctrl Measure Register */
#define BMP280_CONFIG_REG				(0xF5)                                 /* Configuration Register */
#define BMP280_PRESSURE_MSB_REG			(0xF7)                                 /* Pressure MSB Register */
#define BMP280_PRESSURE_LSB_REG			(0xF8)                                 /* Pressure LSB Register */
#define BMP280_PRESSURE_XLSB_REG		(0xF9)                                 /* Pressure XLSB Register */
#define BMP280_TEMPERATURE_MSB_REG		(0xFA)                                 /* Temperature MSB Reg */
#define BMP280_TEMPERATURE_LSB_REG		(0xFB)                                 /* Temperature LSB Reg */
#define BMP280_TEMPERATURE_XLSB_REG		(0xFC)                                 /* Temperature XLSB Reg */

#define BMP280_SLEEP_MODE				(0x00)
#define BMP280_FORCED_MODE				(0x01)
#define BMP280_NORMAL_MODE				(0x03)

#define BMP280_TEMPERATURE_CALIB_DIG_T1_LSB_REG             (0x88)
#define BMP280_PRESSURE_TEMPERATURE_CALIB_DATA_LENGTH       (24)
#define BMP280_DATA_FRAME_SIZE			(6)

#define BMP280_OVERSAMP_SKIPPED			(0x00)
#define BMP280_OVERSAMP_1X				(0x01)
#define BMP280_OVERSAMP_2X				(0x02)
#define BMP280_OVERSAMP_4X				(0x03)
#define BMP280_OVERSAMP_8X				(0x04)
#define BMP280_OVERSAMP_16X				(0x05)
			 
u8 iicDevReadByte(u8 devaddr,u8 addr);	                                       /*读一字节*/
void iicDevWriteByte(u8 devaddr,u8 addr,u8 data);	                           /*写一字节*/
void iicDevRead(u8 devaddr,u8 addr,u8 len,u8 *rbuf);                           /*连续读取多个字节*/
void iicDevWrite(u8 devaddr,u8 addr,u8 len,u8 *wbuf);                          /*连续写入多个字节*/

bool BMP280Init(void);

void BMP280GetData(float* pressure, float* temperature, float* asl);
float BMP_Data(void);

extern float alt; // 高度

#endif
