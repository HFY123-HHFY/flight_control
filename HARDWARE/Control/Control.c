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
static float gyro_bias_z = -6.4f; // 陀螺仪Z轴零偏(原始LSB)

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
    float raw_output; // PID计算的原始输出，未限幅

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

    // 计算输出
    raw_output = pid->kp * pid->error0 + pid->ki * pid->error_sum + pid->kd * (pid->error0 - pid->error1);
    pid->output = Limit_Output(raw_output, pid->Out_max);

    return pid->output;
}

/*
    控制层
    入口参数：Pitch 俯仰角，Roll 横滚角，Yaw 偏航角，alt 高度 （！均为实际值）
*/
void PID_OUT(float Pitch, float Roll, float Yaw, float alt) 
{
    float pitch_rate_target = 0.0f; // Pitch外环输出的目标角速度
    float roll_rate_target = 0.0f; // Roll外环输出的目标角速度
    float yaw_rate_target = 0.0f; // Yaw外环输出的目标角速度

    float gyro_pitch_dps = 0.0f; // 陀螺仪测量的Pitch轴角速度(deg/s)
    float gyro_roll_dps = 0.0f; // 陀螺仪测量的Roll轴角速度(deg/s)
    float gyro_yaw_dps = 0.0f; // 陀螺仪测量的Yaw轴角速度(deg/s)

    float pid_pitch_rate_output = 0.0f; // Pitch内环PID输出
    float pid_roll_rate_output = 0.0f; // Roll内环PID输出
    float pid_yaw_rate_output = 0.0f; // Yaw内环PID输出
    float pid_alt_output = 0.0f; // Altitude内环PID输出

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
    // 外环(角度)输出目标角速度
    pitch_rate_target = Limit_Output(PID_Calc(&pid_pitch, Pitch), RATE_TARGET_MAX_DPS);
    roll_rate_target = Limit_Output(PID_Calc(&pid_roll, Roll), RATE_TARGET_MAX_DPS);
    yaw_rate_target = Limit_Output(PID_Calc(&pid_yaw, Yaw), RATE_TARGET_MAX_DPS);

    // 内环(角速度)实际值: 由陀螺仪原始值换算为 deg/s
    gyro_pitch_dps = GyroRawToDps(gyrox, gyro_bias_x);
    gyro_roll_dps = GyroRawToDps(gyroy, gyro_bias_y);
    gyro_yaw_dps = GyroRawToDps(gyroz, gyro_bias_z);

    // 内环(角速度)目标值: 外环输出的目标角速度
    pid_rate_pitch.Target = pitch_rate_target;
    pid_rate_roll.Target = roll_rate_target;
    pid_rate_yaw.Target = yaw_rate_target;

    // 内环输出用于电机混控
    pid_pitch_rate_output = Limit_Output(PID_Calc(&pid_rate_pitch, gyro_pitch_dps), Motor_out_max);
    pid_roll_rate_output = Limit_Output(PID_Calc(&pid_rate_roll, gyro_roll_dps), Motor_out_max);
    pid_yaw_rate_output = Limit_Output(PID_Calc(&pid_rate_yaw, gyro_yaw_dps), Motor_out_max);
    pid_alt_output = Limit_Output(PID_Calc(&pid_alt, alt), Motor_out_max);

    // 更新 PID 输出
    pid_pitch.output = pid_pitch_rate_output;
    pid_roll.output = pid_roll_rate_output;
    pid_yaw.output = pid_yaw_rate_output;
    pid_alt.output = pid_alt_output;
}

/*
    执行层
*/
void PID_right_out(void)
{
    if (pid_flag == 1)
    {
        uint16_t m1;
        uint16_t m2;
        uint16_t m3;
        uint16_t m4;

        pid_enabled = 1; //启用PID计算
        PID_OUT(Pitch, Roll, Yaw, alt);

        // 四路混控应一次性下发为一帧，避免后续调用把前面通道覆盖为0
        m1 = speed_temp + pid_pitch.output + pid_roll.output;
        m2 = speed_temp - pid_pitch.output + pid_roll.output;
        m3 = speed_temp + pid_pitch.output - pid_roll.output;
        m4 = speed_temp - pid_pitch.output - pid_roll.output;
        TIM1_DShot_Write(m1, m2, m3, m4);
    }
    else if (pid_flag == 0)
    {
        pid_enabled = 0; //禁用PID计算
        pid_alt.output = 0;
        pid_pitch.output = 0;
        pid_roll.output = 0;
        pid_yaw.output = 0;
        pid_rate_pitch.output = 0;
        pid_rate_roll.output = 0;
        pid_rate_yaw.output = 0;
        TIM1_DShot_Write(0, 0, 0, 0);
    }
}

/*
    Pitch 和 Roll 合并双环控制函数
    入口参数：
    actual_pitch: Pitch实际角度
    actual_roll:  Roll实际角度
*/
void PID_Pitch_Roll_Combined(float actual_pitch, float actual_roll)
{
    float pitch_out = 0.0f; // Pitch内环PID输出
    float roll_out = 0.0f; // Roll内环PID输出
    float pitch_rate_target = 0.0f; // Pitch外环输出的目标角速度
    float roll_rate_target = 0.0f; // Roll外环输出的目标角速度
    float gyro_pitch_dps = 0.0f; // 陀螺仪测量的Pitch轴角速度(deg/s)
    float gyro_roll_dps = 0.0f; // 陀螺仪测量的Roll轴角速度(deg/s)

    if (pid_task_flag == 1)
    {
        pid_task_flag = 0; // 清除PID中断标志

        // 设置外环目标角度
        pid_pitch.Target = 0.0f;
        pid_roll.Target = 0.0f;
        
        //外环PID输出
        pitch_rate_target = Limit_Output(PID_Calc(&pid_pitch, actual_pitch), RATE_TARGET_MAX_DPS);
        roll_rate_target = Limit_Output(PID_Calc(&pid_roll, actual_roll), RATE_TARGET_MAX_DPS);

        // 内环实际角速度: 由陀螺仪原始值换算为 deg/s
        gyro_roll_dps = GyroRawToDps(gyrox, gyro_bias_x);
        gyro_pitch_dps = GyroRawToDps(gyroy, gyro_bias_y);

        // 内环目标角速度 = 外环PID输出
        pid_rate_pitch.Target = pitch_rate_target;
        pid_rate_roll.Target = roll_rate_target;

        // pid_rate_pitch.Target= 0.0f;
        // pid_rate_roll.Target = 0.0f;

        // 内环PID输出
        pitch_out = Limit_Output(PID_Calc(&pid_rate_pitch, gyro_pitch_dps), Motor_out_max);
        roll_out  = Limit_Output(PID_Calc(&pid_rate_roll, gyro_roll_dps), Motor_out_max);
        
        //串级PID最终输出
        pid_pitch.output = pitch_out;
        pid_roll.output = roll_out;
				
         //加载输出到电机上
        Motor_Test();

        // if (print_task_flag)
		// {
		// 	print_task_flag = 0;
		// 	printf("%.1f,%.1f,%.1f,%.1f,%.1f\n",pid_pitch.Target, Pitch, pid_rate_pitch.Target,gyro_pitch_dps,pid_pitch.output);
		// }
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

// 设置陀螺仪零偏(原始LSB)
void Set_Gyro_Bias(float bias_x, float bias_y, float bias_z)
{
    gyro_bias_x = bias_x;
    gyro_bias_y = bias_y;
    gyro_bias_z = bias_z;
}
