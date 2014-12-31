#ifndef __PROJ_H__
#define __PROJ_H__

#include <msp430.h>
#include <stdlib.h>
#include "lm4780_helper.h"
#include "config.h"

#define MCLK_FREQ           1048576

// software i2c config
#define I2C_MASTER_DIR P1DIR
#define I2C_MASTER_OUT P1OUT
#define I2C_MASTER_IN  P1IN
#define I2C_MASTER_SCL BIT2
#define I2C_MASTER_SDA BIT3

#define I2C_TX_BUFF_LEN     8
uint8_t i2c_tx_buff[I2C_TX_BUFF_LEN];

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

#define MUTE_FRONT      P2OUT |= BIT0
#define UNMUTE_FRONT    P2OUT &= ~BIT0

#define MUTE_REAR       P2OUT |= BIT1
#define UNMUTE_REAR     P2OUT &= ~BIT1

#define TEMP_LEN 64
char str_temp[TEMP_LEN];

void main_init(void);
void sleep(void);
void wake_up(void);
void check_events(void);
void check_ir(void);

#define FLASH_ADDR          SEGMENT_B
void settings_init(uint8_t * addr);
void settings_apply(void);

uint8_t str_to_uint16(char *str, uint16_t * out, const uint8_t seek,
     const uint8_t len, const uint16_t min, const uint16_t max);

#define DETECT_CHANNELS 2   // we have 2 detection circuits - one for front speakers and one for the rear
#define ON_DEBOUNCE     6    // channel is unmuted if sound is detected this many times in a row
#define OFF_DEBOUNCE    600  // channel is muted if sound is not detected this many times in a row
//#define ALL_INPUTS      0x82

extern const uint8_t detect_port[DETECT_CHANNELS];

struct ampy_stat_t {
    uint8_t input_last[DETECT_CHANNELS];   // [bool] true if channel was muted
    uint16_t count[DETECT_CHANNELS];    // [uint] debounce counter
};

struct ampy_stat_t stat;

static const struct ampy_settings_t defaults = {
    A_VER,      // struct version
    0x3,        // snd_detect
    0           // mute_flag
};


#endif
