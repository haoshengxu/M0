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
extern volatile uint32_t imu_frame_count;

int main(void)
{
    SYSCFG_DL_init();
    IMU601_init();
    LineSensor_init();

    /* OLED 初始化 */
    OLED_Init();
    OLED_ColorTurn(0);
    OLED_DisplayTurn(0);
    OLED_Clear();

    UART_send_string(PRINT_INST, "System Ready\r\n");

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
        char buf[200];
        sprintf(buf,
                "LS:%4u %4u %4u %4u %4u %4u %4u %4u | BLK:%d%d%d%d%d%d%d%d\r\n",
                ls[0], ls[1], ls[2], ls[3], ls[4], ls[5], ls[6], ls[7],
                (black_line>>0)&1, (black_line>>1)&1,
                (black_line>>2)&1, (black_line>>3)&1,
                (black_line>>4)&1, (black_line>>5)&1,
                (black_line>>6)&1, (black_line>>7)&1);
        UART_send_string(PRINT_INST, buf);

        /* ---- OLED 显示 IMU 数据 ---- */
        char oled_str[30];

        sprintf(oled_str, "Yaw:%.1f", current_attitude.yaw);
        OLED_ShowString(0, 0, (u8 *)oled_str, 16);

        sprintf(oled_str, "Pt:%.1f Rl:%.1f", current_attitude.pitch, current_attitude.roll);
        OLED_ShowString(0, 16, (u8 *)oled_str, 16);

        /* 黑线状态显示 */
        sprintf(oled_str, "BLK:%d%d%d%d%d%d%d%d",
                (black_line>>0)&1, (black_line>>1)&1,
                (black_line>>2)&1, (black_line>>3)&1,
                (black_line>>4)&1, (black_line>>5)&1,
                (black_line>>6)&1, (black_line>>7)&1);
        OLED_ShowString(0, 32, (u8 *)oled_str, 16);

        OLED_Refresh();

        delay_ms(200);
    }
}
