#include "Contol_task.h"

float yaw_fused = 0.0f; // 融合后的航向角

// 全局变量
static volatile uint16_t g_fusion_dt_ms = 0;
static volatile uint8_t g_mpu_data_ready = 0;
static volatile uint16_t g_qmc_poll_ms = 0;

static volatile float g_yaw_fused = 0.0f;
static volatile float g_fusion_dt_s = 0.001f;

// MPU6050陀螺仪原始数据
static YawKalman_t g_yaw_kf;

 // 外部变量
static float NormalizeAngle180(float angle)
{
  while (angle > 180.0f) angle -= 360.0f;
  while (angle < -180.0f) angle += 360.0f;
  return angle;
}

// 磁力计和陀螺仪数据融合，返回融合后的航向角
static void YawKalman_Init(YawKalman_t *kf, float init_yaw_deg)
{
  kf->yaw = NormalizeAngle180(init_yaw_deg);
  kf->bias = 0.0f;

  kf->P00 = 1.0f;
  kf->P01 = 0.0f;
  kf->P10 = 0.0f;
  kf->P11 = 1.0f;

  kf->Q_yaw  = 0.02f;
  kf->Q_bias = 0.003f;
  kf->R_mag  = 3.0f;

  kf->inited = 1;
}

// 更新卡尔曼滤波器，返回融合后的航向角
static float YawKalman_Update(YawKalman_t *kf, float gyro_z_dps, float yaw_mag_deg, float dt_s)
{
  float rate;
  float P00, P01, P10, P11;
  float y, S, K0, K1;
  float nP00, nP01, nP10, nP11;

  yaw_mag_deg = NormalizeAngle180(yaw_mag_deg);

  if (!kf->inited)
  {
      YawKalman_Init(kf, yaw_mag_deg);
      return kf->yaw;
  }

  rate = gyro_z_dps - kf->bias;
  kf->yaw = NormalizeAngle180(kf->yaw + rate * dt_s);

  P00 = kf->P00 + dt_s * (dt_s * kf->P11 - kf->P01 - kf->P10 + kf->Q_yaw);
  P01 = kf->P01 - dt_s * kf->P11;
  P10 = kf->P10 - dt_s * kf->P11;
  P11 = kf->P11 + kf->Q_bias * dt_s;

  kf->P00 = P00;
  kf->P01 = P01;
  kf->P10 = P10;
  kf->P11 = P11;

  y = NormalizeAngle180(yaw_mag_deg - kf->yaw);
  S = kf->P00 + kf->R_mag;
  K0 = kf->P00 / S;
  K1 = kf->P10 / S;

  kf->yaw  = NormalizeAngle180(kf->yaw + K0 * y);
  kf->bias = kf->bias + K1 * y;

  nP00 = (1.0f - K0) * kf->P00;
  nP01 = (1.0f - K0) * kf->P01;
  nP10 = kf->P10 - K1 * kf->P00;
  nP11 = kf->P11 - K1 * kf->P01;

  kf->P00 = nP00;
  kf->P01 = nP01;
  kf->P10 = nP10;
  kf->P11 = nP11;

  return kf->yaw;
}

// 卡尔曼滤波器初始化函数
void ControlTask_FusionInit(void)
{
  g_fusion_dt_ms = 1;
  g_mpu_data_ready = 0;
  g_qmc_poll_ms = 20;

  Angle_XY = QMC_Data();
  YawKalman_Init(&g_yaw_kf, Angle_XY);

  g_yaw_fused = Angle_XY;
  g_fusion_dt_s = 0.001f;
}
 
// 卡尔曼滤波器更新函数
void ControlTask_FusionUpdate(void)
{
  uint16_t dt_ms;
  uint8_t need_qmc_update = 0;
  float dt_s;
  float gyro_z_dps;

  if (g_mpu_data_ready == 0)
  {
      return;
  }

  __disable_irq();
  g_mpu_data_ready = 0;
  dt_ms = g_fusion_dt_ms;
  g_fusion_dt_ms = 0;

  if (g_qmc_poll_ms >= 20)
  {
      g_qmc_poll_ms = 0;
      need_qmc_update = 1;
  }
  __enable_irq();

  if (dt_ms == 0)
  {
      dt_ms = 1;
  }

  dt_s = 0.001f * (float)dt_ms;
  g_fusion_dt_s = dt_s;

  mpu_dmp_get_data(&Pitch, &Roll, &Yaw);
  MPU_Get_Gyroscope(&gyrox, &gyroy, &gyroz);

  if (need_qmc_update || (!g_yaw_kf.inited))
  {
      Angle_XY = QMC_Data();
  }

  gyro_z_dps = (float)gyroz / YAW_GYRO_Z_SCALE;
  g_yaw_fused = YawKalman_Update(&g_yaw_kf, gyro_z_dps, Angle_XY, dt_s);
}

// 获取融合后的航向角，单位为度，范围为[-180, 180]
float ControlTask_GetYawFused(void)
{
  return g_yaw_fused;
}

float ControlTask_GetFusionDtS(void)
{
  return g_fusion_dt_s;
}

// 定时器2中断服务函数
void TIM2_IRQHandler(void)
{

  if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
  {
    if (g_fusion_dt_ms < 1000)
    {
        g_fusion_dt_ms++;
    }

    if (g_qmc_poll_ms < 1000)
    {
        g_qmc_poll_ms++;
    }
  }
  TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
}

// MPU6050外部中断处理函数
void EXTI9_5_IRQHandler(void)
{
  if (EXTI_GetITStatus(EXTI_Line7) != RESET)
  {
    g_mpu_data_ready = 1;
    mpu_flag = 1; // 设置MPU6050数据更新标志
  }
  EXTI_ClearITPendingBit(EXTI_Line7);
}

// // QMC5883外部中断处理函数
// void EXTI0_IRQHandler(void)
// {
//   if (EXTI_GetITStatus(EXTI_Line0) != RESET)
//   {
//     EXTI_ClearITPendingBit(EXTI_Line0);
//   }
// }

