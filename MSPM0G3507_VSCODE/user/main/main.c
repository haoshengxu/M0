/**
 * @file main.c
 * @brief MSPM0G3507 IMU601 陀螺仪 + 八路灰度循迹 + OLED 显示 + LED1 闪烁
 *
 * 接线:
 *   IMU601  → MSPM0G3507
 *   T (TX)  → PB13 (UART3 RX)
 *   R (RX)  → PA26 (UART3 TX)
 *
 *   循迹传感器 → MSPM0G3507
 *   AD0      → PA31
 *   AD1      → PA12
 *   AD2      → PB08
 *   ADC(OUT) → PA27
 *   EN       → PB27
 *
 *   OLED (I2C) → MSPM0G3507
 *   SCL        → PB2
 *   SDA        → PB3
 *
 *   USB-TTL → MSPM0G3507
 *   RX      → PA10 (UART0 TX)
 */

#include "ti_msp_dl_config.h"
#include "imu601.h"
#include "uart.h"
#include "line_sensor.h"
#include "oled.h"
#include "motor.h"
#include "encoder.h"
#include <stdio.h>

/* oled.c 需要 delay_ms */
void delay_ms(uint32_t ms)
{
    for (uint32_t i = 0; i < ms; i++) {
        delay_cycles(80000);
    }
}

extern Attitude_t current_attitude;
extern volatile uint32_t imu_rx_count;
extern volatile uint32_t g_irq1_count;
extern volatile uint32_t imu_frame_count;

int main(void)
{
    SYSCFG_DL_init();
    IMU601_init();
    LineSensor_init();
    Motor_Init();
    Encoder_Init();

    /* 启用 GPIOB 中断（编码器 COUNT） */
    NVIC_EnableIRQ(1);  /* GPIOB 所有中断（编码器 COUNT） */

    /* OLED 初始化 — 暂时关闭排查问题 */
    // OLED_Init();
    // OLED_ColorTurn(0);
    // OLED_DisplayTurn(0);
    // OLED_Clear();

    /* 电机测试：A 5% 正转，B 5% 正转 */
    MotorA_SetSpeed(0);
    MotorB_SetSpeed(0);

    UART_send_string(PRINT_INST, "System Ready\r\n");

    int32_t last_enc1 = 0, last_enc2 = 0;
    #define LOOP_MS  200

    while (1)
    {
        DL_GPIO_togglePins(LED1_PORT, LED1_PIN_22_PIN);

        /* 读取循迹传感器 */
        uint16_t ls[8];
        LineSensor_readAll(ls);

        /* 黑线检测 */
        #define BLACK_THRESHOLD  1500
        uint8_t black_line = 0;
        for (int i = 0; i < 8; i++) {
            if (ls[i] < BLACK_THRESHOLD) black_line |= (1 << i);
        }

        /* ---- 串口输出 ---- */
        int32_t e1 = g_enc1_count, e2 = g_enc2_count;
        int32_t d1 = e1 - last_enc1, d2 = e2 - last_enc2;
        last_enc1 = e1; last_enc2 = e2;

        char buf[200];
        sprintf(buf,
                "E1:%ld(%ldrpm) E2:%ld(%ldrpm)\r\n",
                (long)e1, (long)Encoder_MotorRPM(d1, LOOP_MS),
                (long)e2, (long)Encoder_MotorRPM(d2, LOOP_MS));
        UART_send_string(PRINT_INST, buf);

        /* ---- OLED 暂时关闭 ---- */

        delay_ms(LOOP_MS);
    }
}
