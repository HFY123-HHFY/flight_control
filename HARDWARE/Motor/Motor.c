#include "Motor.h"

/* 电机基础油门值 - 由摇杆提供 */
uint16_t speed_temp = 0;

/* 电机输出值 - 用于存储最终的DShot油门值 */
uint16_t Motor_Output[4] = {DSHOT_THROTTLE_MIN, DSHOT_THROTTLE_MIN, DSHOT_THROTTLE_MIN, DSHOT_THROTTLE_MIN};

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

// 混控反饱和: 先整体平移4路输出，再做限幅，减少高油门时姿态控制失真
static void Motor_MixWithDesaturation(float base, float pitch, float roll,
                                      uint16_t* out_m1, uint16_t* out_m2,
                                      uint16_t* out_m3, uint16_t* out_m4)
{
    float m1_raw = 0.0f; // 电机1的原始输出（尚未限幅）
    float m2_raw = 0.0f; // 电机2的原始输出（尚未限幅）
    float m3_raw = 0.0f; // 电机3的原始输出（尚未限幅）
    float m4_raw = 0.0f; // 电机4的原始输出（尚未限幅）
    float max_raw = 0.0f; // 四路中的最大原始输出
    float min_raw = 0.0f; // 四路中的最小原始输出
    float shift = 0.0f; // 统一平移量，用于将四路一起搬回可用区间

    // 1) 先按混控矩阵得到四路“理想输出”（尚未限幅）
    m1_raw = base + pitch + roll;
    m2_raw = base - pitch + roll;
    m3_raw = base + pitch - roll;
    m4_raw = base - pitch - roll;

    // 2) 找到四路中的最大值和最小值，用于判断是否超出DShot范围
    max_raw = m1_raw;
    if (m2_raw > max_raw) { max_raw = m2_raw; }
    if (m3_raw > max_raw) { max_raw = m3_raw; }
    if (m4_raw > max_raw) { max_raw = m4_raw; }

    min_raw = m1_raw;
    if (m2_raw < min_raw) { min_raw = m2_raw; }
    if (m3_raw < min_raw) { min_raw = m3_raw; }
    if (m4_raw < min_raw) { min_raw = m4_raw; }

    // 3) 计算统一平移量 shift
    //    目标: 尽量把四路一起“搬回”可用区间 [DSHOT_THROTTLE_MIN, DSHOT_THROTTLE_MAX]
    //    这样可以保留四路之间的差分关系(姿态控制力矩)，避免单路先截断引入偏航/侧偏。
    shift = 0.0f;
    if (max_raw > (float)DSHOT_THROTTLE_MAX)
    {
        shift = (float)DSHOT_THROTTLE_MAX - max_raw;
    }
    if ((min_raw + shift) < (float)DSHOT_THROTTLE_MIN)
    {
        shift += (float)DSHOT_THROTTLE_MIN - (min_raw + shift);
    }

    // 4) 对四路统一加 shift，再做最终限幅
    m1_raw += shift;
    m2_raw += shift;
    m3_raw += shift;
    m4_raw += shift;

    *out_m1 = Motor_DShotClamp(m1_raw);
    *out_m2 = Motor_DShotClamp(m2_raw);
    *out_m3 = Motor_DShotClamp(m3_raw);
    *out_m4 = Motor_DShotClamp(m4_raw);
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

        // 使用“反饱和混控”: 在高基础油门下尽量保持PID差分有效
        Motor_MixWithDesaturation((float)speed_temp,
                                  pid_rate_pitch.output,
                                  pid_rate_roll.output,
                                  &m1, &m2, &m3, &m4);
        Motor_Output[0] = m1;
        Motor_Output[1] = m2;
        Motor_Output[2] = m3;
        Motor_Output[3] = m4;
        TIM1_DShot_Write(m1, m2, m3, m4);
        LED1 = 0;
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

            Motor_Output[0] = m1;
            Motor_Output[1] = m2;
            Motor_Output[2] = m3;
            Motor_Output[3] = m4;
        }
        TIM1_DShot_Write(m1, m2,m3, m4);
        LED1 = 1;
    }
}
