/**
 * @file encoder.c
 * @brief 编码器驱动（GPIO 双沿中断 + 正交解码）
 *
 * MSPM0: 所有 GPIO 中断共用 GROUP1 (IRQ 1)
 */
#include "encoder.h"

volatile int32_t g_enc1_count = 0;
volatile int32_t g_enc2_count = 0;
volatile uint32_t g_irq1_count = 0;
volatile uint32_t g_pin_irq[4] = {0}; /* E1A, E1B, E2A, E2B 各自触发次数 */

void Encoder_Init(void)
{
    g_enc1_count = 0;
    g_enc2_count = 0;
}

/* 所有 GPIO 中断（PB4=E2A, PB5=E2B, PB12=E1A, PB23=E1B） */
void GROUP1_IRQHandler(void)
{
    g_irq1_count++;
    uint32_t mis = GPIOB->CPU_INT.MIS;

    if (mis & E1A_PIN_E1A_PIN) { g_pin_irq[0]++;
        if (DL_GPIO_readPins(GPIOB, E1A_PIN_E1A_PIN)) { /* A 上升沿 */
            if (DL_GPIO_readPins(GPIOB, E1B_PIN_E1B_PIN)) g_enc1_count--;
            else g_enc1_count++;
        } else { /* A 下降沿 */
            if (DL_GPIO_readPins(GPIOB, E1B_PIN_E1B_PIN)) g_enc1_count++;
            else g_enc1_count--;
        }
    }
    if (mis & E1B_PIN_E1B_PIN) { g_pin_irq[1]++;
        if (DL_GPIO_readPins(GPIOB, E1B_PIN_E1B_PIN)) { /* B 上升沿 */
            if (DL_GPIO_readPins(GPIOB, E1A_PIN_E1A_PIN)) g_enc1_count++;
            else g_enc1_count--;
        } else { /* B 下降沿 */
            if (DL_GPIO_readPins(GPIOB, E1A_PIN_E1A_PIN)) g_enc1_count--;
            else g_enc1_count++;
        }
    }
    if (mis & E2A_PIN_E2A_PIN) { g_pin_irq[2]++;
        if (DL_GPIO_readPins(GPIOB, E2A_PIN_E2A_PIN)) {
            if (DL_GPIO_readPins(GPIOB, E2B_PIN_E2B_PIN)) g_enc2_count++;  /* 反转 */
            else g_enc2_count--;
        } else {
            if (DL_GPIO_readPins(GPIOB, E2B_PIN_E2B_PIN)) g_enc2_count--;
            else g_enc2_count++;
        }
    }
    if (mis & E2B_PIN_E2B_PIN) { g_pin_irq[3]++;
        if (DL_GPIO_readPins(GPIOB, E2B_PIN_E2B_PIN)) {
            if (DL_GPIO_readPins(GPIOB, E2A_PIN_E2A_PIN)) g_enc2_count--;  /* 反转 */
            else g_enc2_count++;
        } else {
            if (DL_GPIO_readPins(GPIOB, E2A_PIN_E2A_PIN)) g_enc2_count++;
            else g_enc2_count--;
        }
    }
    DL_GPIO_clearInterruptStatus(GPIOB, mis);
}
