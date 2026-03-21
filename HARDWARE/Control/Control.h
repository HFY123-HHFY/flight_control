#ifndef __CONTROL_H
#define __CONTROL_H

#include "math.h"
#include "Motor.h"
#include "NVIC_Int.h"
#include "bmp280.h"

#define Motor_out_max 4200	//PID输出最后加载到电机上的输出限幅

// PID
typedef struct 
{
    float kp;
    float ki;
    float kd;

    float Target; // 目标值
    float Actual; // 实际值
    float output; // 输出

    float error0; // 当前误差
    float error1; // 上次误差
    float error_sum; // 误差的累加

    int16_t Integral_max;//积分最大值
    int16_t Out_max;//输出最大值
} PID_TypeDef;

void PID_Contorl_Init(void); // PID控制初始化
void Set_PID(PID_TypeDef* pid, float kp, float ki, float kd); // 设置PID参数
void Set_Target_Attitude(float target_pitch, float target_roll, float target_yaw, float target_alt); // 设置目标姿态

extern PID_TypeDef pid_pitch, pid_roll, pid_yaw, pid_alt; // 全局PID变量，4个姿态角
extern int pid_flag; // PID控制标志位

#endif
