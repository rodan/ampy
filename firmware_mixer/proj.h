#ifndef __PROJ_H__
#define __PROJ_H__

#include <msp430.h>
#include <stdlib.h>
#include "config.h"

#define pga_enable          P1OUT |= BIT0
#define pga_disable         P1OUT &= ~BIT0
#define led_on              P6OUT |= BIT0
#define led_off             P6OUT &= ~BIT0

#define VERSION             1   // must be incremented if struct settings_t changes
#define FLASH_ADDR          SEGMENT_B

#define true                1
#define false               0

void main_init(void);
void settings_init(uint8_t * addr);
void wake_up(void);
void check_events(void);
void display_help(void);
void show_settings(void);

void mute_pga(const uint8_t pga_id, const uint8_t save);
void unmute_pga(const uint8_t pga_id, const uint8_t save);
void set_volume(const uint8_t pga_id, const uint8_t vol_right,
                const uint8_t vol_left, const uint8_t save,
                const uint8_t autounmute);




uint8_t str_to_uint16(char *str, uint16_t * out, const uint8_t seek,
                      const uint8_t len, const uint16_t min,
                      const uint16_t max);

//cannot use PXOUT due to gcc querkiness, so feed the addresses instead
//P1OUT == 0x202
//P2OUT == 0x203
//P4OUT == 0x223

// chip select port location
//static const uint16_t CS_OUT[6] = {P4OUT, P1OUT, P4OUT, P2OUT, P1OUT, P1OUT};
static const uint16_t CS_OUT[6] = { 0x223, 0x202, 0x223, 0x203, 0x202, 0x202 };
static const uint8_t CS_PORT[6] = { BIT7, BIT6, BIT2, BIT0, BIT4, BIT2 };

// mute port location
//static const uint16_t MUTE_OUT[6] = {P4OUT, P1OUT, P4OUT, P1OUT, P1OUT, P1OUT};
static const uint16_t MUTE_OUT[6] =
    { 0x223, 0x202, 0x223, 0x202, 0x202, 0x202 };
static const uint8_t MUTE_PORT[6] = { BIT6, BIT5, BIT0, BIT7, BIT3, BIT1 };

struct settings_t {
    uint8_t ver;                // firmware version
    uint8_t mute_flag;
    uint8_t v1_r;
    uint8_t v1_l;
    uint8_t v2_r;
    uint8_t v2_l;
    uint8_t v3_r;
    uint8_t v3_l;
    uint8_t v4_r;
    uint8_t v4_l;
    uint8_t v5_r;
    uint8_t v5_l;
    uint8_t v6_r;
    uint8_t v6_l;
};

static const struct settings_t defaults = {
    VERSION,                    // ver
    0,                          // mute_flag
    0,                          // v1_r
    0,                          // v1_l
    0,                          // v2_r
    0,                          // v2_l
    0,                          // v3_r
    0,                          // v3_l
    0,                          // v4_r
    0,                          // v4_l
    0,                          // v5_r
    0,                          // v5_l
    0,                          // v6_r
    0,                          // v6_l
};

#endif
