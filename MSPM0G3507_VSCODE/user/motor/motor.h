/**
 * @file motor.h
 * @brief TB6612 双路电机驱动
 *
 * 引脚:
 *   AIN1 = PA13, AIN2 = PB26, PWMA = PA28 (TIMA1 CCP0)
 *   BIN1 = PB09, BIN2 = PB07, PWMB = PB20 (TIMA0 CCP1)
 *
 * TB6612 控制逻辑:
 *   IN1=1 IN2=0 → 正转
 *   IN1=0 IN2=1 → 反转
 *   IN1=0 IN2=0 → 刹车
 *   PWM = 占空比控制转速
 */

#ifndef MOTOR_H
#define MOTOR_H

#include "ti_msp_dl_config.h"
#include <stdint.h>

/* speed 范围: -100 ~ 100，负值反转，0 停止 */
void MotorA_SetSpeed(int8_t speed);
void MotorB_SetSpeed(int8_t speed);
void Motor_Stop(void);
void Motor_Init(void);

#endif /* MOTOR_H */
