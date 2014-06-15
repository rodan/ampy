#ifndef __PROJ_H__
#define __PROJ_H__

#include <msp430.h>
#include <stdlib.h>
#include "config.h"
#include "pga2311_helper.h"

#define pga_enable          P1OUT |= BIT0
#define pga_disable         P1OUT &= ~BIT0
#define led_on              P6OUT |= BIT0
#define led_off             P6OUT &= ~BIT0

#define I2C_USE_DEV         0
#define I2C_CLK_SRC         2
#define I2C_CLK_DIV         8
#define I2C_SLAVE_ADDR      0x28

#define VERSION             1   // must be incremented if struct settings_t changes
#define FLASH_ADDR          SEGMENT_B

void main_init(void);
void settings_init(uint8_t * addr);
void wake_up(void);
void check_events(void);


static const struct mixer_settings_t defaults = {
    VERSION,                    // ver
    10,                          // mute_flag
    20,                          // v1_r
    30,                          // v1_l
    40,                          // v2_r
    50,                          // v2_l
    60,                          // v3_r
    70,                          // v3_l
    80,                          // v4_r
    90,                          // v4_l
    100,                          // v5_r
    110,                          // v5_l
    120,                          // v6_r
    130,                          // v6_l
};

#endif
