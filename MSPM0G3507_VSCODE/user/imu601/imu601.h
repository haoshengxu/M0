#ifndef IMU601_H
#define IMU601_H

#include "ti_msp_dl_config.h"
#include "../uart/uart.h"

#include <stdint.h>

typedef struct {
    float yaw;
    float pitch;
    float roll;
} Attitude_t;

/*
 * 接线:
 *   汇电籽-601       MSPM0G3507
 *   V                5V
 *   G                GND
 *   T (TX)           PA25 (UART3 RX)
 *   R (RX)           PA26 (UART3 TX)
 */

void IMU601_init(void);

#endif /* IMU601_H */
