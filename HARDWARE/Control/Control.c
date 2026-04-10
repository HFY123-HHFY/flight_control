#include "Control.h"

// 1开，0关
uint8_t pid_flag = 0; // pid 控制标志，1启用，0禁用
uint8_t pid_enabled = 1; //PID计算标志，1启用，0禁用

uint8_t pid_task_flag = 0; // pid中断标志，1表示定时器中断触发，0表示未触发

/*
    目标层
    Pitch 俯仰角，Roll 横滚角，Yaw 偏航角，altitude 高度
*/
float Target_Pitch = 0.0f; // 设定目标俯仰角
float Target_Roll = 0.0f; // 设定目标横滚角
float Target_Yaw = 0.0f; // 设定目标偏航角
float Target_Alt = 0.0f; //设定目标高度

static float gyro_bias_x = -32.1f; // 陀螺仪X轴零偏(原始LSB)
static float gyro_bias_y = -3.1f; // 陀螺仪Y轴零偏(原始LSB)
// static float gyro_bias_z = -6.4f; // 陀螺仪Z轴零偏(原始LSB)

// 将陀螺仪原始值转换为角速度（deg/s）
static float GyroRawToDps(short raw, float bias)
{
    return ((float)raw - bias) / GYRO_SENS_2000DPS;
}

// 限幅函数
float Limit_Output(float value, float max)
{
    return (value > max) ? max : ((value < -max) ? -max : value);
}

// PID初始化
void PID_Init(PID_TypeDef* pid, float kp, float ki, float kd)
{
    // memset(pid, 0, sizeof(PID_TypeDef)); // 清零结构体
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;

    pid->Target = 0;
    pid->Actual = 0;
    pid->output = 0;

    pid->error0 = 0;
    pid->error1 = 0;
    pid->error_sum = 0;

    pid->P_out = 0;
    pid->I_out = 0;
    pid->D_out = 0;
    pid->Integral_max = 1000; // 积分最大值
    pid->Out_max = 2047; // 输出最大值
}

// 全局PID变量，外环:角度/高度，内环:角速度
PID_TypeDef pid_pitch, pid_roll, pid_yaw, pid_alt;
PID_TypeDef pid_rate_pitch, pid_rate_roll, pid_rate_yaw;

// PID参数初始化
void PID_Contorl_Init(void)
{
    PID_Init(&pid_pitch, 0.0f, 0.00f, 0.0f);
    PID_Init(&pid_roll, 0.0f, 0.00f, 0.0f);
    PID_Init(&pid_yaw, 0.0f, 0.00f, 0.0f);
    PID_Init(&pid_alt, 0.0f, 0.00f, 0.0f);

    PID_Init(&pid_rate_pitch, 0.0f, 0.00f, 0.0f);
    PID_Init(&pid_rate_roll, 0.0f, 0.00f, 0.0f);
    PID_Init(&pid_rate_yaw, 0.0f, 0.00f, 0.0f);
}

// PID计算
float PID_Calc(PID_TypeDef* pid, float Actual)
{
    float pif_output;

    pid->Actual = Actual;
    pid->error1 = pid->error0;
    pid->error0 = pid->Target - pid->Actual;

    if (pid->ki >= 0.00001f)
    {
        pid->error_sum += pid->error0;
        pid->error_sum = (pid->error_sum > pid->Integral_max) ? pid->Integral_max : ((pid->error_sum < -pid->Integral_max) ? -pid->Integral_max : pid->error_sum);
    }
    else 
    {
        pid->error_sum = 0;
    }

    pid->P_out = pid->kp * pid->error0;
    pid->I_out = pid->ki * pid->error_sum;
    pid->D_out = pid->kd * (pid->error0 - pid->error1);

    // 计算输出
    pif_output = pid->P_out + pid->I_out + pid->D_out;

    pid->output = pif_output;

    return pid->output;
}

void lowpass_filter(float* input, float* output, float alpha)
{
    static float prev_output = 0.0f;
    *output = alpha * (*input) + (1 - alpha) * prev_output;
    prev_output = *output;
}

/*
    Pitch 和 Roll 合并双环控制函数
    入口参数：
    actual_pitch: Pitch实际角度
    actual_roll:  Roll实际角度
*/
void PID_Pitch_Roll_Combined(float actual_pitch, float actual_roll)
{
    float pitch_rate_out = 0.0f; // Pitch内环PID输出
    float roll_rate_out = 0.0f; // Roll内环PID输出
    float pitch_rate_target = 0.0f; // Pitch外环输出
    float roll_rate_target = 0.0f; // Roll外环输出
    float gyro_pitch_dps = 0.0f; // 陀螺仪测量的Pitch轴角速度(deg/s)
    float gyro_roll_dps = 0.0f; // 陀螺仪测量的Roll轴角速度(deg/s)

    if (pid_task_flag == 1)
    {
        pid_task_flag = 0; // 清除PID中断标志

        // 设置外环目标角度
        pid_pitch.Target = 0.0f;
        pid_roll.Target = 0.0f;
        
        //外环PID输出
        pitch_rate_target = PID_Calc(&pid_pitch, actual_pitch);
        roll_rate_target = PID_Calc(&pid_roll, actual_roll);

        // 内环实际角速度: 由陀螺仪原始值换算为 deg/s
        gyro_roll_dps = GyroRawToDps(gyrox, gyro_bias_x);
        gyro_pitch_dps = GyroRawToDps(gyroy, gyro_bias_y);

        lowpass_filter(&gyro_roll_dps, &gyro_roll_dps, 0.5f); // 对Roll角速度进行低通滤波
        lowpass_filter(&gyro_pitch_dps, &gyro_pitch_dps, 0.5f); // 对Pitch角速度进行低通滤波

        // 内环目标角速度 = 外环PID输出
        pid_rate_pitch.Target = pitch_rate_target;
        pid_rate_roll.Target = roll_rate_target;

        // 内环PID输出
        pitch_rate_out = PID_Calc(&pid_rate_pitch, gyro_pitch_dps);
        roll_rate_out  = PID_Calc(&pid_rate_roll, gyro_roll_dps);
        
        //串级PID最终输出
        pid_rate_pitch.output = pitch_rate_out;
        pid_rate_roll.output = roll_rate_out;
				
         //加载输出到电机上
        Motor_Test();
    }
}

// 光流速度环控制
void LC307_Speed_Control(void)
{
    // 光流速度环入口-20ms
    if (lc307_speed_task_flag)
    {
        lc307_speed_task_flag = 0;
        //......预留速度环逻辑
    }
}

// 光流位置环控制
void LC307_Position_Control(void)
{
    // 光流位置环入口-40ms
    if (lc307_pos_task_flag)
    {
        lc307_pos_task_flag = 0;
        //......预留位置环逻辑
    }
}

// 设置PID参数
void Set_PID(PID_TypeDef* pid, float kp, float ki, float kd)
{
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
}
