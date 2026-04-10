#include "Motor.h"

uint16_t speed_temp = 0;

// 将PID输出和基础油门值转换为DShot油门值，并进行限幅
static uint16_t Motor_DShotClamp(float val)
{
    if (val < (float)DSHOT_THROTTLE_MIN)
    {
        return DSHOT_THROTTLE_MIN;
    }
    if (val > (float)DSHOT_THROTTLE_MAX)
    {
        return DSHOT_THROTTLE_MAX;
    }
    return (uint16_t)val;
}

// 将当前油门值降低到最小值
static uint16_t Motor_RampDownToMin(uint16_t current, uint16_t step)
{
    if (current <= DSHOT_THROTTLE_MIN)
    {
        return DSHOT_THROTTLE_MIN;
    }

    if (current > (uint16_t)(DSHOT_THROTTLE_MIN + step))
    {
        return (uint16_t)(current - step);
    }

    return DSHOT_THROTTLE_MIN;
}

void Motor_Test(void)
{
    static uint16_t m1 = DSHOT_THROTTLE_MIN;
    static uint16_t m2 = DSHOT_THROTTLE_MIN;
    static uint16_t m3 = DSHOT_THROTTLE_MIN;
    static uint16_t m4 = DSHOT_THROTTLE_MIN;

    if (Key == 1)
    {
        /* 
            电机1: 基础 + Pitch调节 + Roll调节
            电机2: 基础 - Pitch调节 + Roll调节
            电机3: 基础 + Pitch调节 - Roll调节
            电机4: 基础 - Pitch调节 - Roll调节
        */

        m1 = Motor_DShotClamp((float)speed_temp + pid_rate_pitch.output + pid_rate_roll.output);
        m2 = Motor_DShotClamp((float)speed_temp - pid_rate_pitch.output + pid_rate_roll.output);
        m3 = Motor_DShotClamp((float)speed_temp + pid_rate_pitch.output - pid_rate_roll.output);
        m4 = Motor_DShotClamp((float)speed_temp - pid_rate_pitch.output - pid_rate_roll.output);
        TIM1_DShot_Write(m1, m2, m3, m4);
    }
    else if (Key == 2)
    {
        pid_rate_pitch.output = 0;
        pid_rate_roll.output = 0;

        const uint16_t ramp_step = 8U; // 每次调用降低4个单位的油门，2次调用降低16个单位/秒的下降速率
        static uint8_t ramp_div = 0U; // 降速计数器

        ramp_div++;
        if (ramp_div >= 2U)
        {
            ramp_div = 0U;

            m1 = Motor_RampDownToMin(m1, ramp_step);
            m2 = Motor_RampDownToMin(m2, ramp_step);
            m3 = Motor_RampDownToMin(m3, ramp_step);
            m4 = Motor_RampDownToMin(m4, ramp_step);
        }
        TIM1_DShot_Write(m1, m2,m3, m4);
    }
}
