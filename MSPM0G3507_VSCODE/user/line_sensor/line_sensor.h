/**
 * @file line_sensor.h
 * @brief 感为八路灰度传感器驱动
 *
 * 引脚:
 *   AD0 = PA31, AD1 = PA12, AD2 = PB08  (74HC4051 通道选择)
 *   ADC = PA27  (模拟输入)
 *   EN  = PB27  (传感器使能, 低有效)
 */

#ifndef LINE_SENSOR_H
#define LINE_SENSOR_H

#include "ti_msp_dl_config.h"
#include <stdint.h>

#define LINE_SENSOR_CHANNELS  8

/* 全局可见，方便调试 watch */
extern volatile uint16_t g_ls_values[8];

/* 初始化传感器 GPIO（AD0/1/2/EN） */
void LineSensor_init(void);

/* 读取指定通道 (0~7) 的 ADC 原始值 */
uint16_t LineSensor_readChannel(uint8_t ch);

/* 一次读取全部 8 通道，存入 values[8] */
void LineSensor_readAll(uint16_t values[8]);

#endif /* LINE_SENSOR_H */
