#include "Control.h"

// 1开，0关
int pid_flag = 0; // pid 控制标志，1启用，0禁用
int pid_enabled = 1; //PID计算标志，1启用，0禁用

uint8_t pid_task_flag = 0; // pid中断标志，1表示定时器中断触发，0表示未触发

/*
    目标层
    Pitch 俯仰角，Roll 横滚角，Yaw 偏航角，altitude 高度
*/
float Target_Pitch = 0.0f; // 设定目标俯仰角
float Target_Roll = 0.0f; // 设定目标横滚角
float Target_Yaw = 0.0f; // 设定目标偏航角
float Target_Alt = 0.0f; //设定目标高度

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

    pid->Integral_max = 1000; // 积分最大值
    pid->Out_max = 200; // 输出最大值
}

// 全局PID变量，4个姿态角
PID_TypeDef pid_pitch, pid_roll, pid_yaw, pid_alt;

// PID参数初始化
void PID_Contorl_Init(void)
{
    PID_Init(&pid_pitch, 0.0f, 0.00f, 0.0f);
    PID_Init(&pid_roll, 0.0f, 0.00f, 0.0f);
    PID_Init(&pid_yaw, 0.0f, 0.00f, 0.0f);
    PID_Init(&pid_alt, 0.0f, 0.00f, 0.0f);
}

// PID计算
float PID_Calc(PID_TypeDef* pid, float Actual)
{
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
    
    //计算输出
    float raw_output = pid->kp * pid->error0 + pid->ki * pid->error_sum + pid->kd * (pid->error0 - pid->error1);

    // 卡尔曼滤波
    static float kalman_output = 0; // 卡尔曼滤波后的输出
    static float P = 1.0f;          // 估计误差协方差
    const float Q = 0.1f;          // 过程噪声协方差，越大响应越快，但是滤波效果会变差
    const float R = 0.01f;           // 测量噪声协方差，和Q反之
    float K;                        // 卡尔曼增益，权重

    // 预测更新
    P = P + Q;

    // 计算卡尔曼增益
    K = P / (P + R);

    // 更新估计值
    kalman_output = kalman_output + K * (raw_output - kalman_output);

    // 更新误差协方差
    P = (1 - K) * P;

    // 低通滤波
    const float alpha = 0.8f; // 滤波系数，0.0~1.0，越接近1越平滑
    pid->output = alpha * pid->output + (1.0f - alpha) * kalman_output;

    return pid->output;
}

// 限幅函数
float Limit_Output(float value, float max)
{
    return (value > max) ? max : ((value < -max) ? -max : value);
}

/*
    控制层
    入口参数：Pitch 俯仰角，Roll 横滚角，Yaw 偏航角，alt 高度 （！均为实际值）
*/
void PID_OUT(float Pitch, float Roll, float Yaw, float alt) 
{
    if (!pid_enabled)
    {
        return;
    }
    /*
    传入目标值
    */
    pid_pitch.Target = Target_Pitch; // pithch目标值
    pid_roll.Target = Target_Roll; //  roll目标值
    pid_yaw.Target = Target_Yaw; //  yaw目标值
    pid_alt.Target = Target_Alt; // alt目标值

    /*
        传入实际值，计算得到输出值
    */
    // 计算 PID 输出
    float pid_pitch_output = Limit_Output(PID_Calc(&pid_pitch, Pitch), Motor_out_max);
    float pid_roll_output = Limit_Output(PID_Calc(&pid_roll, Roll), Motor_out_max);
    float pid_yaw_output = Limit_Output(PID_Calc(&pid_yaw, Yaw), Motor_out_max);
    float pid_alt_output = Limit_Output(PID_Calc(&pid_alt, alt), Motor_out_max);//........

    // 更新 PID 输出
    pid_pitch.output = pid_pitch_output;
    pid_roll.output = pid_roll_output;
    pid_yaw.output = pid_yaw_output;
    pid_alt.output = pid_alt_output;
}

/*
    执行层
*/
void PID_right_out(void)
{
    if (pid_flag == 1)
    {
        pid_enabled = 1; //启用PID计算
        PID_OUT(Pitch, Roll, Yaw, alt);//................
        Motor_Control(1, pid_alt.output + pid_pitch.output - pid_roll.output + pid_yaw.output);
        Motor_Control(2, pid_alt.output + pid_pitch.output + pid_roll.output - pid_yaw.output);
        Motor_Control(3, pid_alt.output - pid_pitch.output + pid_roll.output + pid_yaw.output);
        Motor_Control(4, pid_alt.output - pid_pitch.output - pid_roll.output - pid_yaw.output);
    }
    else if (pid_flag == 0)
    {
        pid_enabled = 0; //禁用PID计算
        pid_alt.output = 0;
        pid_pitch.output = 0;
        pid_roll.output = 0;
        pid_yaw.output = 0;
        Motor_Control(1, 0);
        Motor_Control(2, 0);
        Motor_Control(3, 0);
        Motor_Control(4, 0);
    }
}

// 设置PID参数
void Set_PID(PID_TypeDef* pid, float kp, float ki, float kd)
{
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
}

// 设置目标姿态
void Set_Target_Attitude(float target_pitch, float target_roll, float target_yaw, float target_alt)
{
    Target_Pitch = target_pitch;
    Target_Roll = target_roll;
    Target_Yaw = target_yaw;
    Target_Alt = target_alt;
}

/*
    Pitch 和 Roll 合并双环控制函数
    入口参数：
    actual_pitch: Pitch实际角度
    actual_roll:  Roll实际角度
*/
void PID_Pitch_Roll_Combined(float actual_pitch, float actual_roll)
{
    if (pid_task_flag == 1)
    {
        pid_task_flag = 0; // 清除PID中断标志

        // 设置目标值为0
        pid_pitch.Target = 0.0f;
        pid_roll.Target = 0.0f;
        
        // 计算 PID 输出
        float pitch_out = Limit_Output(PID_Calc(&pid_pitch, actual_pitch), Motor_out_max);
        float roll_out  = Limit_Output(PID_Calc(&pid_roll, actual_roll), Motor_out_max);
        
        pid_pitch.output = pitch_out; // 更新 PID 输出到结构体
        pid_roll.output = roll_out; // 更新 PID 输出到结构体

        Motor_Test(); //加载输出到电机上
    }
}
