#ifndef __CONTROL_TASK_H
#define __CONTROL_TASK_H

#include "Bsp.h"

// 卡尔曼滤波器状态结构体
typedef struct
{
    float yaw;
    float bias;
    float P00;
    float P01;
    float P10;
    float P11;
    float Q_yaw;
    float Q_bias;
    float R_mag;
    uint8_t inited;
} YawKalman_t;

#define YAW_GYRO_Z_SCALE 16.4f   // MPU6050陀螺仪量程若为±2000dps，则灵敏度16.4LSB/(deg/s)

void ControlTask_FusionInit(void); // 卡尔曼滤波器初始化
void ControlTask_FusionUpdate(void); // 卡尔曼滤波器更新

float ControlTask_GetYawFused(void); // 获取融合后的航向角，单位为度，范围为[-180, 180]
float ControlTask_GetFusionDtS(void); // 获取两次融合更新之间的时间间隔，单位为秒

extern float yaw_fused; // 融合后的航向角

#endif
