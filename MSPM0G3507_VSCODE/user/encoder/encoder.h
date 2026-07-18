/**
 * @file encoder.h
 * @brief 编码器驱动（GPIO 中断 + 软件计数）
 *
 * A/B 正交信号 → 双沿捕获实现 4 倍分辨率
 * 接线:
 *   E1A = PB12, E1B = PB23  (电机 A)
 *   E2A = PB04, E2B = PB05  (电机 B)
 */

#ifndef ENCODER_H
#define ENCODER_H

#include "ti_msp_dl_config.h"
#include <stdint.h>

/* 全局脉冲计数（中断中累加，可被主循环读取） */
extern volatile int32_t g_enc1_count;
extern volatile int32_t g_enc2_count;
extern volatile uint32_t g_irq1_count;
extern volatile uint32_t g_pin_irq[4];  /* E1A, E1B, E2A, E2B */

/* 13 PPR × 4x 解码 = 52 脉冲/电机转一圈，28:1 减速比 = 1456 脉冲/车轮转一圈 */
#define ENC_PPR           13
#define ENC_MULT           4
#define ENC_GEAR_RATIO    28
#define ENC_CPR_MOTOR     (ENC_PPR * ENC_MULT)           /* 52 */
#define ENC_CPR_WHEEL     (ENC_CPR_MOTOR * ENC_GEAR_RATIO) /* 1456 */

static inline int32_t Encoder_MotorRPM(int32_t delta, uint32_t dt_ms) {
    return (int32_t)((int64_t)delta * 60000 / (ENC_CPR_MOTOR * (int32_t)dt_ms));
}
static inline int32_t Encoder_WheelRPM(int32_t delta, uint32_t dt_ms) {
    return (int32_t)((int64_t)delta * 60000 / (ENC_CPR_WHEEL * (int32_t)dt_ms));
}

void Encoder_Init(void);
void GROUP1_IRQHandler(void);  /* GPIOB 中断共享入口 */

#endif /* ENCODER_H */
