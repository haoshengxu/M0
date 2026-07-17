#include "imu601.h"

/* 全局变量：当前姿态 */
Attitude_t current_attitude;

/* 内部延时（80MHz） */
static void delay_ms(uint32_t ms)
{
    for (uint32_t i = 0; i < ms; i++) {
        delay_cycles(80000);
    }
}

void IMU601_init(void)
{
    /* 复位指令（固定） */
    const uint8_t IMU_reset[] = {0xAA, 0x55, 0x60, 0x12, 0x00, 0x72};
    UART_send_buffer(IMU_INST, IMU_reset, sizeof(IMU_reset));
    delay_ms(300);

    /* 校准指令（需根据实际情况微调） */
    const uint8_t IMU_cali[] = {
        0xAA, 0x55, 0x60, 0x14, 0x04, 0xCD, 0x4C, 0xB4, 0x43, 0x88
    };
    UART_send_buffer(IMU_INST, IMU_cali, sizeof(IMU_cali));
    delay_ms(300);

    NVIC_EnableIRQ(IMU_INST_INT_IRQN);
}

/* ---- 协议解析 ---- */
static uint8_t  IMU_RX_buffer[12] = {0};
static uint8_t  IMU_RX_index   = 0;

/* 数据帧示例: AA 55 60 01 06 E0 8C 18 04 2C F7 36 */

static void parse_attitude_only(const uint8_t *payload, Attitude_t *out)
{
    uint16_t yaw_raw   = (payload[1] << 8) | payload[0];
    int16_t  pitch_raw = (int16_t)((payload[3] << 8) | payload[2]);
    int16_t  roll_raw  = (int16_t)((payload[5] << 8) | payload[4]);
    out->yaw   = yaw_raw   / 100.0f;
    out->pitch = pitch_raw / 100.0f;
    out->roll  = roll_raw  / 100.0f;
}

static void parse_imu601_data(void)
{
    uint8_t checksum = 0;
    for (int i = 2; i < 11; i++) {
        checksum += IMU_RX_buffer[i];
    }
    if (checksum == IMU_RX_buffer[11]) {
        parse_attitude_only(&IMU_RX_buffer[5], &current_attitude);
    }
}

/* 调试：记录 UART3 收到的字节数 */
volatile uint32_t imu_rx_count = 0;
volatile uint32_t imu_frame_count = 0;

/* UART3 中断处理 */
void IMU_INST_IRQHandler(void)
{
    uint32_t stat = DL_UART_getPendingInterrupt(IMU_INST);

    if (stat == DL_UART_IIDX_RX) {
        uint8_t byte = DL_UART_receiveData(IMU_INST);
        imu_rx_count++;

        if (IMU_RX_index == 0) {
            if (byte == 0xAA) {
                IMU_RX_buffer[0] = byte;
                IMU_RX_index = 1;
            }
        } else if (IMU_RX_index == 1) {
            if (byte == 0x55) {
                IMU_RX_buffer[1] = byte;
                IMU_RX_index = 2;
            } else {
                IMU_RX_index = 0;
            }
        } else {
            IMU_RX_buffer[IMU_RX_index] = byte;
            IMU_RX_index++;

            if (IMU_RX_index >= 12) {
                IMU_RX_index = 0;
                imu_frame_count++;
                parse_imu601_data();
            }
        }
    }
}
