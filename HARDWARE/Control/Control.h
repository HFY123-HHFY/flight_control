#ifndef __CONTROL_H
#define __CONTROL_H

#include "math.h"
#include "Motor.h"
#include "NVIC_Int.h"
#include "QMC5883P.h"
#include "bmp280.h"
#include "LC307.h"

#define Motor_out_max 200	//PID输出最后加载到电机上的输出限幅
#define GYRO_SENS_2000DPS 16.4f // ±2000dps量程下灵敏度: 16.4 LSB/(deg/s)
#define RATE_TARGET_MAX_DPS 200.0f // 外环输出的目标角速度限幅(deg/s)

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

/*
mpu6050
*/
void PID_Pitch_Roll_Combined(float actual_pitch, float actual_roll); // Pitch 和 Roll 合并双环控制函数
void Set_Gyro_Bias(float bias_x, float bias_y, float bias_z); // 设置陀螺仪零偏(原始LSB)

/*
LC307光流
*/
void LC307_Speed_Control(void); // 光流速度环控制
void LC307_Position_Control(void); // 光流位置环控制

extern PID_TypeDef pid_pitch, pid_roll, pid_yaw, pid_alt; // 外环PID变量（角度/高度）
extern PID_TypeDef pid_rate_pitch, pid_rate_roll, pid_rate_yaw; // 内环PID变量（角速度）
extern uint8_t pid_flag; // PID控制标志位
extern uint8_t pid_task_flag; // PID中断标志

#endif
