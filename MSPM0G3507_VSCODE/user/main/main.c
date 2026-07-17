/**
 * @file main.c
 * @brief MSPM0G3507 IMU601 陀螺仪读取 + 串口打印 + LED1 闪烁
 *
 * 接线:
 *   IMU601  → MSPM0G3507
 *   T (TX)  → PA25 (UART3 RX)
 *   R (RX)  → PA26 (UART3 TX)
 *
 *   USB-TTL → MSPM0G3507
 *   RX      → PA28 (UART0 TX)
 */

#include "ti_msp_dl_config.h"
#include "imu601.h"
#include "uart.h"
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

    UART_send_string(PRINT_INST, "IMU601 Ready\r\n");

    while (1)
    {
        DL_GPIO_togglePins(LED1_PORT, LED1_PIN_22_PIN);

        char buf[100];
        sprintf(buf, "RX:%lu Frm:%lu Y:%.1f P:%.1f R:%.1f\r\n",
                imu_rx_count,
                imu_frame_count,
                current_attitude.yaw,
                current_attitude.pitch,
                current_attitude.roll);
        UART_send_string(PRINT_INST, buf);

        delay_ms(200);
    }
}
