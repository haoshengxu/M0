/**
 * @file motor.c
 * @brief TB6612 双路电机驱动实现
 *  PWM 反相: INIT_VAL_LOW → CCR=1000-占空比
 */
#include "motor.h"
#include <stdlib.h>

#define PWM_PERIOD  1000  /* 80MHz / 1000 = 80kHz */

static void set_pwm_a(uint16_t ccr) {
    if (ccr > PWM_PERIOD) ccr = PWM_PERIOD;
    DL_TimerA_setCaptureCompareValue(PWMA_INST, ccr, DL_TIMERA_CAPTURE_COMPARE_0_INDEX);
}
static void set_pwm_b(uint16_t ccr) {
    if (ccr > PWM_PERIOD) ccr = PWM_PERIOD;
    DL_TimerA_setCaptureCompareValue(PWMB_INST, ccr, DL_TIMERA_CAPTURE_COMPARE_1_INDEX);
}

void Motor_Init(void)
{
    DL_GPIO_clearPins(AIN1_PORT, AIN1_PIN_13_PIN);
    DL_GPIO_clearPins(AIN2_PORT, AIN2_PIN_26_PIN);
    DL_GPIO_clearPins(BIN1_PORT, BIN1_PIN_9_PIN);
    DL_GPIO_clearPins(BIN2_PORT, BIN2_PIN_7_PIN);

    /* 先把所有 CCR 设到 PWM_PERIOD（0% 占空比）再启动定时器 */
    DL_TimerA_setCaptureCompareValue(PWMA_INST, PWM_PERIOD,
        DL_TIMERA_CAPTURE_COMPARE_0_INDEX);
    DL_TimerA_setCaptureCompareValue(PWMB_INST, PWM_PERIOD,
        DL_TIMERA_CAPTURE_COMPARE_0_INDEX);
    DL_TimerA_setCaptureCompareValue(PWMB_INST, PWM_PERIOD,
        DL_TIMERA_CAPTURE_COMPARE_1_INDEX);

    DL_TimerA_startCounter(PWMA_INST);
    DL_TimerA_startCounter(PWMB_INST);
}

void MotorA_SetSpeed(int8_t speed)
{
    int32_t pct = (speed > 0) ? speed : -speed;
    uint16_t ccr = PWM_PERIOD - (uint16_t)(pct * PWM_PERIOD / 100);

    if (speed > 0) {
        DL_GPIO_setPins(AIN1_PORT, AIN1_PIN_13_PIN);
        DL_GPIO_clearPins(AIN2_PORT, AIN2_PIN_26_PIN);
    } else if (speed < 0) {
        DL_GPIO_clearPins(AIN1_PORT, AIN1_PIN_13_PIN);
        DL_GPIO_setPins(AIN2_PORT, AIN2_PIN_26_PIN);
    } else {
        DL_GPIO_clearPins(AIN1_PORT, AIN1_PIN_13_PIN);
        DL_GPIO_clearPins(AIN2_PORT, AIN2_PIN_26_PIN);
        ccr = PWM_PERIOD;
    }
    set_pwm_a(ccr);
}

void MotorB_SetSpeed(int8_t speed)
{
    int32_t pct = (speed > 0) ? speed : -speed;
    uint16_t ccr = (uint16_t)(pct * PWM_PERIOD / 100);  /* EDGE_ALIGN_UP 不需反相 */

    if (speed > 0) {
        DL_GPIO_setPins(BIN1_PORT, BIN1_PIN_9_PIN);
        DL_GPIO_clearPins(BIN2_PORT, BIN2_PIN_7_PIN);
    } else if (speed < 0) {
        DL_GPIO_clearPins(BIN1_PORT, BIN1_PIN_9_PIN);
        DL_GPIO_setPins(BIN2_PORT, BIN2_PIN_7_PIN);
    } else {
        DL_GPIO_clearPins(BIN1_PORT, BIN1_PIN_9_PIN);
        DL_GPIO_clearPins(BIN2_PORT, BIN2_PIN_7_PIN);
        ccr = 0;  /* CCR=0 → INIT_VAL_LOW → 0% 占空比 */
    }
    set_pwm_b(ccr);
}

void Motor_Stop(void)
{
    MotorA_SetSpeed(0);
    MotorB_SetSpeed(0);
}
