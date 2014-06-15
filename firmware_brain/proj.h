#ifndef __PROJ_H__
#define __PROJ_H__

#include <msp430.h>
#include <stdlib.h>
#include "config.h"

#define MCLK_FREQ           1048576

// hardware i2c config
#define I2C_USE_DEV         0
#define I2C_CLK_SRC         2
#define I2C_CLK_DIV         12

// infrared config
#define IR_SEL P1SEL
#define IR_DIR P1DIR
#define IR_IN  P1IN
#define IR_PIN BIT4

void main_init(void);
void sleep(void);
void wake_up(void);
void check_events(void);
void check_ir(void);

uint8_t str_to_uint16(char *str, uint16_t * out, const uint8_t seek,
     const uint8_t len, const uint16_t min, const uint16_t max);

#endif
