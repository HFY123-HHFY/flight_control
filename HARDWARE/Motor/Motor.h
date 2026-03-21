#ifndef __MOTOR_H
#define __MOTOR_H

#include "Pwm.h"
#include "delay.h"
#include "Key.h"

void Motor_Control(int mosX, float duty);
void Motor_Test(void);

extern uint16_t speed_temp;

#endif
