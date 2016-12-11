#ifndef __PROJ_H__
#define __PROJ_H__

#include <msp430.h>
#include <stdlib.h>
#include <inttypes.h>
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
    {20,30,40,50,60,70,80,90,100,110,120,130}, // volumes
    0
};

#endif
