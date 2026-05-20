#include "Control.h"

uint8_t pid_task_flag = 0; // pid中断标志，1表示定时器中断触发，0表示未触发

/*
    目标层
    Pitch 俯仰角，Roll 横滚角，Yaw 偏航角，altitude 高度
*/
float Target_Pitch = 0.0f; // 设定目标俯仰角
float Target_Roll = 0.0f; // 设定目标横滚角
float Target_Yaw = 0.0f; // 设定目标偏航角
float Target_Alt = 0.0f; //设定目标高度

static float gyro_bias_x = -26.0f; // 陀螺仪X轴零偏(原始LSB)
static float gyro_bias_y = -3.0f; // 陀螺仪Y轴零偏(原始LSB)
// static float gyro_bias_z = -6.4f; // 陀螺仪Z轴零偏(原始LSB)
static const float attitude_deadband_deg = 0.3f; // 姿态软死区宽度(单位:deg)
static const uint16_t gyro_bias_calib_samples = 1000U; // 2秒自动校准(500Hz节拍下1000点)
static uint16_t gyro_bias_sample_count = 0U; // 采样计数器，累计采样点数，达到gyro_bias_calib_samples后计算零偏并标记校准完成
static float gyro_bias_acc_x = 0.0f; // 累积陀螺仪X轴原始值的和，用于计算零偏
static float gyro_bias_acc_y = 0.0f; // 累积陀螺仪Y轴原始值的和，用于计算零偏
static uint8_t gyro_bias_ready = 0U; // 零偏准备就绪标志，0表示正在校准，1表示校准完成并可使用零偏值

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

// PID状态重置，清除误差和输出
static void PID_ResetState(PID_TypeDef* pid)
{
    pid->error0 = 0.0f;
    pid->error1 = 0.0f;
    pid->error_sum = 0.0f;

    pid->P_out = 0.0f;
    pid->I_out = 0.0f;
    pid->D_out = 0.0f;
    pid->output = 0.0f;
}

// 软死区: 死区内输出0, 死区外平滑减去死区值，避免硬切换带来的突变
static float ApplySoftDeadband(float value, float deadband)
{
    if (value > deadband)
    {
        return value - deadband;
    }
    if (value < -deadband)
    {
        return value + deadband;
    }
    return 0.0f;
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
    float output_limited;
    float err_sum_candidate;
    uint8_t integral_hold = 0;

    pid->Actual = Actual;
    pid->error1 = pid->error0;
    pid->error0 = pid->Target - pid->Actual;

    if (pid->ki < 0.00001f)
    {
        pid->error_sum = 0;
    }

    err_sum_candidate = pid->error_sum;
    if (pid->ki >= 0.00001f)
    {
        err_sum_candidate += pid->error0;
        err_sum_candidate = (err_sum_candidate > pid->Integral_max) ? pid->Integral_max : ((err_sum_candidate < -pid->Integral_max) ? -pid->Integral_max : err_sum_candidate);
    }

    if (pid->error0 < 0.2f && pid->error0 > -0.2f)
    {
        // 小误差区域缓慢释放积分，保留配平能力，避免频繁清零导致慢偏
        err_sum_candidate *= 0.98f;
    }

    pid->P_out = pid->kp * pid->error0;
    pid->I_out = pid->ki * err_sum_candidate;
    pid->D_out = pid->kd * (pid->error0 - pid->error1);

    // 计算输出
    pif_output = pid->P_out + pid->I_out + pid->D_out;

    // 输出饱和且误差继续推动同方向饱和时，抑制积分，降低高油门慢偏风险
    output_limited = Limit_Output(pif_output, (float)pid->Out_max);
    if ((output_limited >= (float)pid->Out_max && pid->error0 > 0.0f) ||
        (output_limited <= -(float)pid->Out_max && pid->error0 < 0.0f))
    {
        integral_hold = 1;
    }

    if (integral_hold == 0)
    {
        pid->error_sum = err_sum_candidate;
    }
    pid->I_out = pid->ki * pid->error_sum;
    pif_output = pid->P_out + pid->I_out + pid->D_out;

    pid->output = Limit_Output(pif_output, (float)pid->Out_max);

    return pid->output;
}

// 低通滤波器
static float lowpass_filter(float input, float* prev_output, float alpha)
{
    float output;

    output = alpha * input + (1.0f - alpha) * (*prev_output);
    *prev_output = output;

    return output;
}

/*
    Pitch 和 Roll 合并双环串级PID控制函数
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
    float pitch_actual_for_pid = 0.0f; // 经过软死区处理后的Pitch实际值，用于PID计算
    float roll_actual_for_pid = 0.0f; // 经过软死区处理后的Roll实际值，用于PID计算

    static float gyro_pitch_lpf = 0.0f; // Pitch轴滤波状态
    static float gyro_roll_lpf = 0.0f; // Roll轴滤波状态
    static uint8_t gyro_dbg_div = 0U; // 调试串口分频: 500Hz控制环下, 25次约50ms
    float pitch_error_deg = 0.0f; // Pitch角误差(单位:deg)，用于软死区处理
    float roll_error_deg = 0.0f; // Roll角误差(单位:deg)，用于软死区处理

    if (pid_task_flag == 1)
    {
        pid_task_flag = 0; // 清除PID中断标志

        // 上电后自动做陀螺仪零偏标定: 仅在未解锁状态下累计样本
        // 若用户提前解锁，则保持默认零偏，避免控制任务被一直阻塞。
        if (gyro_bias_ready == 0U)
        {
            if (Key != 1U)
            {
                gyro_bias_acc_x += (float)gyrox;
                gyro_bias_acc_y += (float)gyroy;
                LED3 = 1;
                gyro_bias_sample_count++;

                if (gyro_bias_sample_count >= gyro_bias_calib_samples)
                {
                    LED3 = 0;
                    Set_Gyro_Bias(gyro_bias_acc_x / (float)gyro_bias_sample_count,
                                  gyro_bias_acc_y / (float)gyro_bias_sample_count,
                                  0.0f);
                    // 校准完成时打印最终零偏(单位:原始LSB)
                    usart_printf(USART3,
                                 "gyro bias ready: bx=%.2f, by=%.2f, n=%u\r\n",
                                 gyro_bias_x,
                                 gyro_bias_y,
                                 (unsigned int)gyro_bias_sample_count);
                    gyro_bias_ready = 1U;
                }
            }
            else
            {
                gyro_bias_ready = 1U;
            }

            // 标定期间不输出姿态修正，避免未完成校准时积分累积导致偏航/侧偏
            PID_ResetState(&pid_pitch);
            PID_ResetState(&pid_roll);
            PID_ResetState(&pid_rate_pitch);
            PID_ResetState(&pid_rate_roll);
            pid_rate_pitch.output = 0.0f;
            pid_rate_roll.output = 0.0f;
            Motor_Test();
            return;
        }

        // 设置外环目标角度
        pid_pitch.Target = Target_Pitch;
        pid_roll.Target = Target_Roll;

        // 对姿态误差做软死区处理: 小角度误差直接归零，越过死区后平滑输出
        pitch_error_deg = ApplySoftDeadband(pid_pitch.Target - actual_pitch, attitude_deadband_deg);
        roll_error_deg = ApplySoftDeadband(pid_roll.Target - actual_roll, attitude_deadband_deg);

        // 把软死区后的误差映射成“等效实际值”，复用现有PID_Calc逻辑
        pitch_actual_for_pid = pid_pitch.Target - pitch_error_deg;
        roll_actual_for_pid = pid_roll.Target - roll_error_deg;
        
        //外环PID输出
        if (pitch_error_deg == 0.0f)
        {
            pitch_rate_target = 0.0f;
            PID_ResetState(&pid_pitch);
        }
        else
        {
            pitch_rate_target = PID_Calc(&pid_pitch, pitch_actual_for_pid);
        }

        if (roll_error_deg == 0.0f)
        {
            roll_rate_target = 0.0f;
            PID_ResetState(&pid_roll);
        }
        else
        {
            roll_rate_target = PID_Calc(&pid_roll, roll_actual_for_pid);
        }

        // 内环实际角速度: 由陀螺仪原始值换算为 deg/s
        gyro_roll_dps = GyroRawToDps(gyrox, gyro_bias_x);
        gyro_pitch_dps = GyroRawToDps(gyroy, gyro_bias_y);

        gyro_roll_dps = lowpass_filter(gyro_roll_dps, &gyro_roll_lpf, 0.5f); // 对Roll角速度进行低通滤波
        gyro_pitch_dps = lowpass_filter(gyro_pitch_dps, &gyro_pitch_lpf, 0.5f); // 对Pitch角速度进行低通滤波

        // 每50ms打印一次: 原始值 + 零偏 + 校准后的角速度(dps)
        // gyro_dbg_div++;
        // if (gyro_dbg_div >= 25U)
        // {
        //     gyro_dbg_div = 0U;
        //     usart_printf(USART3,
        //                  "raw(gx=%hd,gy=%hd) bias(bx=%.2f,by=%.2f) dps(rx=%.3f,ry=%.3f)\r\n",
        //                  gyrox,
        //                  gyroy,
        //                  gyro_bias_x,
        //                  gyro_bias_y,
        //                  gyro_roll_dps,
        //                  gyro_pitch_dps);
        // }

        // 内环目标角速度 = 外环PID输出
        pid_rate_pitch.Target = pitch_rate_target;
        pid_rate_roll.Target = roll_rate_target;

        // 内环PID输出
        if (pitch_error_deg == 0.0f)
        {
            pitch_rate_out = 0.0f;
            PID_ResetState(&pid_rate_pitch);
        }
        else
        {
            pitch_rate_out = PID_Calc(&pid_rate_pitch, gyro_pitch_dps);
        }

        if (roll_error_deg == 0.0f)
        {
            roll_rate_out = 0.0f;
            PID_ResetState(&pid_rate_roll);
        }
        else
        {
            roll_rate_out  = PID_Calc(&pid_rate_roll, gyro_roll_dps);
        }
        
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

void Set_Gyro_Bias(float bias_x, float bias_y, float bias_z)
{
    gyro_bias_x = bias_x;
    gyro_bias_y = bias_y;
    (void)bias_z;
}
