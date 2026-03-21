#ifndef __QMC_H
#define __QMC_H

#include <stdint.h>
#include "usart.h"
#include "My_I2C.h"
#include "math.h"

#define QMC5883L_ADDRESS  		0x1A //	QMC5883L I2C从机地址

#define QMC5883P_REG_XOUT_L		0x00
#define QMC5883P_REG_XOUT_H		0x01
#define QMC5883P_REG_YOUT_L		0x02
#define QMC5883P_REG_YOUT_H		0x03
#define QMC5883P_REG_ZOUT_L		0x04
#define QMC5883P_REG_ZOUT_H		0x05

void QMC_Init(void);
float QMC_Data(void);

extern float Angle_XY;

#endif
