#ifndef __LED_H
#define __LED_H

#include "sys.h"

//LED端口定义
#define LED1 PEout(2) // 绿
#define LED2 PEout(3) // 红
#define LED3 PEout(4) // 蓝

void LED_Init(void);

#endif

//CMskeLists.txt

/*没有位带操作IO翻转方法*/
// static volatile uint8_t g_led1Level = 0U;	/* g_led1Level: 定时器中断中用于翻转 LED1 的软件状态位。 */
//g_led1Level = (g_led1Level == 0U) ? 1U : 0U;
// Enroll_LED_Control(LED1, (g_led1Level != 0U) ? LED_HIGH : LED_LOW);

/*
uint8_t 是无符号的8位整数类型，范围从0到255。它通常用于存储小的整数值或字符数据。
uint16_t 是无符号的16位整数类型，范围从0到65535
uint32_t 是无符号的32位整数类型，范围从0到4294967295。它通常用于存储较大的整数值。

int8_t 是有符号的8位整数类型，范围从-128到127。它通常用于存储小的整数值。
int16_t 是有符号的16位整数类型，范围从-32768到32767。它通常用于存储较大的整数值。
int32_t 是有符号的32位整数类型，范围从-2147483648到2147483647。它通常用于存储更大的整数值。
*/
