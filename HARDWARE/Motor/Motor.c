#include "Motor.h"

uint16_t speed_temp = 0;

static uint16_t Motor_ClampDuty(float duty)
{
    if (duty <= 0.0f)
    {
        return 0;
    }
    if (duty >= (float)PWM_DUTY_MAX)
    {
        return PWM_DUTY_MAX;
    }
    return (uint16_t)duty;
}

//电机测试函数
void Motor_Control(int mosX, float duty)
{
    uint16_t pwm = Motor_ClampDuty(duty);

    switch(mosX)
    {
        case 1:
            MOS1_Control(pwm);
            break;
        case 2:
            MOS2_Control(pwm);
            break;
        case 3:
            MOS3_Control(pwm);
            break;
        case 4:
            MOS4_Control(pwm);
            break;
        default:
            break;
    }
}

void Motor_Test(void)
{
    if (Key == 1)
    {
        Motor_Control(1, speed_temp);
        Motor_Control(2, speed_temp);
        Motor_Control(3, speed_temp);
        Motor_Control(4, speed_temp);
    }
    else if (Key == 2)
    {
        if (speed_temp > 20)
        {
            speed_temp -= 5;
        }
        else
        {
            speed_temp = 0;
        }
        Motor_Control(1, speed_temp);
        Motor_Control(2, speed_temp);
        Motor_Control(3, speed_temp);
        Motor_Control(4, speed_temp);
    }
    else if (Key == 3)
    {
        speed_temp = 0;
        Motor_Control(1, 0);
        Motor_Control(2, 0);
        Motor_Control(3, 0);
        Motor_Control(4, 0);
    }
}
