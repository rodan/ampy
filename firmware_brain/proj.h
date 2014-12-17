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

#define true            1
#define false           0

#define LED_SWITCH      P1OUT ^= BIT0
#define LED_ON          P1OUT |= BIT0
#define LED_OFF         P1OUT &= ~BIT0

#define MUTE_FRONT      P2OUT != BIT0
#define UNMUTE_FRONT    P2OUT &= ~BIT0

#define MUTE_REAR       P2OUT != BIT1
#define UNMUTE_REAR     P2OUT &= ~BIT1

#define TEMP_LEN 64
char str_temp[TEMP_LEN];

void main_init(void);
void sleep(void);
void wake_up(void);
void check_events(void);
void check_ir(void);

uint8_t str_to_uint16(char *str, uint16_t * out, const uint8_t seek,
     const uint8_t len, const uint16_t min, const uint16_t max);

struct ampy_stat_t {
    uint8_t snd_f; // sound detected in front channel 
    uint8_t snd_r; // sound detected in back channel
};

struct ampy_stat_t stat;


#endif
