#ifndef __KEY_H
#define __KEY_H

#include "sys.h"
#include "Motor.h"

extern uint8_t Key;

void Key_Init(void);
uint8_t Key_GetNum(void);
void Key_Tick(void);
void key_Get(void);

#endif
