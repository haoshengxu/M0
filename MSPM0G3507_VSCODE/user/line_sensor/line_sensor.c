/**
 * @file line_sensor.c
 * @brief 感为八路灰度传感器驱动（DMA 均值滤波版）
 *
 * ADC AUTO_NEXT 持续采样 → DMA 每个结果搬运一次 → 填满 buffer
 * 切换通道后等 DMA 填满 40 个新样本，均值滤波。
 */

#include "line_sensor.h"
#include <string.h>

volatile uint16_t g_ls_values[8];
static uint16_t g_adc_buffer[40];

static void delay_us(uint32_t us)
{
    for (uint32_t i = 0; i < us; i++) {
        delay_cycles(80);
    }
}

static void select_channel(uint8_t ch)
{
    if (!(ch & 0x01))
        DL_GPIO_setPins(AD0_PORT, AD0_PIN_31_PIN);
    else
        DL_GPIO_clearPins(AD0_PORT, AD0_PIN_31_PIN);

    if (!(ch & 0x02))
        DL_GPIO_setPins(AD1_PORT, AD1_PIN_12_PIN);
    else
        DL_GPIO_clearPins(AD1_PORT, AD1_PIN_12_PIN);

    if (!(ch & 0x04))
        DL_GPIO_setPins(AD2_PORT, AD2_PIN_8_PIN);
    else
        DL_GPIO_clearPins(AD2_PORT, AD2_PIN_8_PIN);
}

void LineSensor_init(void)
{
    DL_GPIO_initPeripheralAnalogFunction(GPIO_ADC12_0_IOMUX_C0);
    DL_GPIO_clearPins(EN_PORT, EN_PIN_27_PIN);

    DL_GPIO_setPins(AD0_PORT, AD0_PIN_31_PIN);
    DL_GPIO_setPins(AD1_PORT, AD1_PIN_12_PIN);
    DL_GPIO_setPins(AD2_PORT, AD2_PIN_8_PIN);

    /* DMA 搬运 40 个 ADC 结果到缓冲区 */
    DL_DMA_setSrcAddr(DMA, DMA_CH0_CHAN_ID,
                      (uint32_t)&ADC0->ULLMEM.MEMRES[0]);
    DL_DMA_setDestAddr(DMA, DMA_CH0_CHAN_ID,
                       (uint32_t)&g_adc_buffer[0]);
    DL_DMA_setTransferSize(DMA, DMA_CH0_CHAN_ID, 40);
    DL_DMA_enableChannel(DMA, DMA_CH0_CHAN_ID);

    DL_ADC12_startConversion(ADC12_0_INST);
    delay_us(100);
}

uint16_t LineSensor_readChannel(uint8_t ch)
{
    if (ch > 7) return 0;

    select_channel(ch);
    delay_us(10); /* 等待 4051 切换稳定 */

    /* 清空缓冲区，等待 DMA 搬运 40 个新样本 */
    memset(g_adc_buffer, 0, sizeof(g_adc_buffer));

    uint32_t timeout = 0;
    while (g_adc_buffer[39] == 0) {
        timeout++;
        /* 超时 100ms 则用寄存器直读兜底 */
        if (timeout > 100000) {
            return DL_ADC12_getMemResult(ADC12_0_INST, ADC12_0_ADCMEM_0);
        }
    }

    /* 均值滤波 */
    uint32_t sum = 0;
    for (uint16_t i = 0; i < 40; i++) {
        sum += g_adc_buffer[i];
    }
    return (uint16_t)(sum / 40);
}

void LineSensor_readAll(uint16_t values[8])
{
    for (uint8_t ch = 0; ch < 8; ch++) {
        values[ch] = LineSensor_readChannel(ch);
        g_ls_values[ch] = values[ch];
    }
}
