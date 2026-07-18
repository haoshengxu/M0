/**
 * @file main.c
 * @brief MSPM0G3507 IMU601 陀螺仪 + 八路灰度循迹 + LED1 闪烁
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
 *   USB-TTL → MSPM0G3507
 *   RX      → PA10 (UART0 TX)
 */

#include "ti_msp_dl_config.h"
#include "imu601.h"
#include "uart.h"
#include "line_sensor.h"
#include <stdio.h>

static void delay_ms(uint32_t ms)
{
    for (uint32_t i = 0; i < ms; i++) {
        delay_cycles(80000);
    }
}

extern Attitude_t current_attitude;
extern volatile uint32_t imu_rx_count;
extern volatile uint32_t imu_frame_count;

int main(void)
{
    SYSCFG_DL_init();
    IMU601_init();
    LineSensor_init();

    UART_send_string(PRINT_INST, "System Ready\r\n");

    while (1)
    {
        DL_GPIO_togglePins(LED1_PORT, LED1_PIN_22_PIN);

        /* 读取八路灰度传感器 */
        uint16_t ls[8];
        LineSensor_readAll(ls);

        /* 黑线检测: ADC < 阈值 → 黑线 (1) */
        #define BLACK_THRESHOLD  1500  /* 根据实际数据调整 */
        uint8_t black_line = 0;
        for (int i = 0; i < 8; i++) {
            if (ls[i] < BLACK_THRESHOLD) {
                black_line |= (1 << i);  /* bit i = 1 表示第 i+1 路检测到黑线 */
            }
        }

        char buf[200];
        sprintf(buf,
                "LS:%4u %4u %4u %4u %4u %4u %4u %4u | "
                "BLK:%d%d%d%d%d%d%d%d\r\n",
                ls[0], ls[1], ls[2], ls[3],
                ls[4], ls[5], ls[6], ls[7],
                (black_line>>0)&1, (black_line>>1)&1,
                (black_line>>2)&1, (black_line>>3)&1,
                (black_line>>4)&1, (black_line>>5)&1,
                (black_line>>6)&1, (black_line>>7)&1);
        UART_send_string(PRINT_INST, buf);

        delay_ms(10);
    }
}
