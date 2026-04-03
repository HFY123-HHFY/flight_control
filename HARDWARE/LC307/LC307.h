#ifndef __LC307_H
#define __LC307_H

#include "stm32f4xx.h" 
#include "delay.h"
#include "usart_1.h"
#include <string.h>
#include "math.h"

// 光流丢失标志位：0表示正常，1表示当前光流数据异常或丢失
extern uint8_t g_lost_pos_dev;

// 光流任务节拍标志位
extern uint8_t lc307_speed_task_flag; // 20ms：速度环节拍
extern uint8_t lc307_pos_task_flag; // 40ms：位置环节拍

// 光流速度（单位：m/s，已做比例换算和补偿）
// g_lc307_speed_x: 机体 X 方向速度
// g_lc307_speed_y: 机体 Y 方向速度
extern float g_lc307_speed_x;
extern float g_lc307_speed_y;

// 光流积分位置（单位：m）
// g_lc307_pos_x: X 方向累计位移
// g_lc307_pos_y: Y 方向累计位移
extern float g_lc307_pos_x;
extern float g_lc307_pos_y;

// 光流速度数据结构：保存光流模块解算后的 X/Y 方向速度
typedef struct
{
	// X 方向光流速度
	float speedX;
	// Y 方向光流速度
	float speedY;
} LC307_Speed_t;

// 启动 LC307 光流数据接收：开启 IDLE 中断和 DMA 持续接收
void Opf_LC307_Start(void);
// 初始化 LC307 光流模块：完成串口通信、协议配置和接收链路准备
uint8_t Opf_LC307_Init(void);

// 设置当前飞行高度/测距值，用于光流速度换算和尺度补偿
void LC307_SetHeight(float distance);
// 设置滤波后的角速度补偿量，用于修正机体转动对光流的影响
void LC307_SetGyroCompensation(float gx_filtered, float gy_filtered);

// 位置积分更新：建议在固定控制周期里调用（例如每 5ms 调用一次，dt_s = 0.005f）
void LC307_UpdatePosition(float dt_s);
// 清零累计位移和积分内部状态（例如解锁/切模式/失效恢复时调用）
void LC307_ResetPosition(void);

// 光流数据接收后的回调入口：驱动层收到一帧有效数据后会调用这里
void getOpticalFlowResult_Callback(float* buf);

#endif
