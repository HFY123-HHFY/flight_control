#ifndef __BUZZER_H
#define __BUZZER_H

#include "sys.h" 
#include "delay.h"

void Buzzer_Init(void);

// duty: 0~1000(千分比), freq: 2500~2900Hz
void Buzzer_Control(uint16_t duty, uint16_t freq);

#endif
