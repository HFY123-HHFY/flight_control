#ifndef __MOTOR_H
#define __MOTOR_H

#include "Pwm.h"
#include "delay.h"
#include "Key.h"
#include "Control.h"
#include "LED.h"

void Motor_Test(void);

extern uint16_t speed_temp;
extern uint16_t Motor_Output[4];

#endif
