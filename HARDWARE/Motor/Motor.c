#include "Motor.h"

uint16_t speed_temp = 0;

static uint16_t Motor_DShotClamp(float val)
{
    if (val <= 0.0f)
    {
        return 0U;
    }
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

void Motor_Test(void)
{
    if (Key == 1)
    {
        uint16_t m1;
        uint16_t m2;
        uint16_t m3;
        uint16_t m4;

        /* 
            电机1: 基础 + Pitch调节 + Roll调节
            电机2: 基础 - Pitch调节 + Roll调节
            电机3: 基础 + Pitch调节 - Roll调节
            电机4: 基础 - Pitch调节 - Roll调节
        */
        m1 = Motor_DShotClamp((float)speed_temp + pid_pitch.output + pid_roll.output);
        m2 = Motor_DShotClamp((float)speed_temp - pid_pitch.output + pid_roll.output);
        m3 = Motor_DShotClamp((float)speed_temp + pid_pitch.output - pid_roll.output);
        m4 = Motor_DShotClamp((float)speed_temp - pid_pitch.output - pid_roll.output);
        TIM1_DShot_Write(m1, m2, m3, m4);
    }
    else if (Key == 2)
    {
        speed_temp = 0;
        TIM1_DShot_Write(speed_temp, speed_temp, speed_temp, speed_temp);
    }
}
