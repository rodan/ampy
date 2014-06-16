
//  audio mixer based on an MSP430F5510
//
//  author:          Petre Rodan <petre.rodan@simplex.ro>
//  available from:  https://github.com/rodan/ampy
//  license:         GNU GPLv3

#include "config.h"

#ifdef USE_UART

#include <stdio.h>
#include <string.h>
#include "proj.h"
#include "drivers/uart1.h"
#include "drivers/timer_a0.h"
#include "drivers/flash.h"
#include "drivers/sys_messagebus.h"
#include "drivers/pga2311.h"
#include "uart_fcts.h"

char str_temp[120];


void display_help(void)
{
    sprintf(str_temp,
            "\r\n --- mixer controller ver %d \r\n",
            VERSION);
    uart1_tx_str(str_temp, strlen(str_temp));

    uart1_tx_str("\r\ncommands\r\n", 12);
    uart1_tx_str("v___   set volume\r\n", 19);
    uart1_tx_str(" ||+ volume - [0-255]\r\n", 23);
    uart1_tx_str(" |+- channel - {r,l,b}\r\n", 24);
    uart1_tx_str(" |   r: right, l: left, b: both\r\n", 33);
    uart1_tx_str(" +-- pga ID - [1-6]\r\n", 21);
    uart1_tx_str("     1: front, 2: rear, 3: line-in\r\n", 36);
    uart1_tx_str("     4: spdif, 5: front-rear pan, 6: center and subwoofer\r\n", 59);
    uart1_tx_str("m_     mute pga\r\n", 17);
    uart1_tx_str(" +- pga ID - [1-6]\r\n", 20);
    uart1_tx_str("u_     unmute pga\r\n", 19);
    uart1_tx_str(" +- pga ID - [1-6]\r\n", 20);
    uart1_tx_str("w_     write current settings to flash\r\n", 40);
    uart1_tx_str(" +- location [1-3]\r\n", 20);
    uart1_tx_str("r_     load saved settings from flash\r\n", 39);
    uart1_tx_str(" +- location [1-3]\r\n", 20);
    uart1_tx_str("s      show settings\r\n", 22);
    uart1_tx_str("?      show help\r\n", 18);
}

void show_settings(void)
{
    uint8_t *ptr;
    uint8_t i;

    ptr = (uint8_t *) &s;

    for (i = 0; i < 14; i++) {
        sprintf(str_temp, "m%d\n", *ptr++);
        uart1_tx_str(str_temp, strlen(str_temp));
        timer_a0_delay(10000);
    }
}

static void uart1_rx_irq(enum sys_message msg)
{
    uint16_t u16;
    uint8_t p;
    uint8_t pga, ch, left = 0, right = 0;
    uint8_t *flash_addr = FLASH_ADDR;
    char *input;
    uint8_t *ptr;
    uint8_t allfine = 0;

    input = (char *)uart1_rx_buf;

    p = input[0];
    if (p > 97) {
        p -= 32;
    }

    if ((p == 63) || (p == M_CMD_HELP)) {       // [h?]
        display_help();
    } else if (p == M_CMD_SHOW) {        // [s]how settings
        show_settings();
        uart1_tx_str("ok\r\n", 4);
    } else if (p == M_CMD_WRITE) {       // [w]rite to flash
        if (str_to_uint16(input, &u16, 1, 1, 1, 3)) {
            if (u16 == 1) {
                flash_addr = SEGMENT_B;
            } else if (u16 == 2) {
                flash_addr = SEGMENT_C;
            } else if (u16 == 3) {
                flash_addr = SEGMENT_D;
            }
            flash_save(flash_addr, (void *)&s, sizeof(s));
            uart1_tx_str("ok\r\n", 4);
        }
    } else if (p == M_CMD_READ) {       // [r]ead from flash
        if (str_to_uint16(input, &u16, 1, 1, 1, 3)) {
            if (u16 == 1) {
                flash_addr = SEGMENT_B;
            } else if (u16 == 2) {
                flash_addr = SEGMENT_C;
            } else if (u16 == 3) {
                flash_addr = SEGMENT_D;
            }
            settings_init(flash_addr);
            uart1_tx_str("ok\r\n", 4);
        }
    } else if (p == M_CMD_MUTE) {       // [m]ute pga
        if (str_to_uint16(input, &u16, 1, 1, 1, 6)) {
            pga = u16;
            pga_mute(pga, 1);
            uart1_tx_str("ok\r\n", 4);
        }
    } else if (p == M_CMD_UNMUTE) {     // [u]nmute pga
        if (str_to_uint16(input, &u16, 1, 1, 1, 6)) {
            pga = u16;
            pga_unmute(pga, 1);
            uart1_tx_str("ok\r\n", 4);
        }
    } else if (p == M_CMD_VOL) {       // [v]olume set
        if (str_to_uint16(input, &u16, 1, 1, 1, 6)) {
            pga = u16;
            ch = input[2];
            if (str_to_uint16(input, &u16, 3, strlen(input) - 3, 0, 255)) {
                ptr = (uint8_t *) & s.v1_r;
                if (ch == 98) {
                    // 'b' - change both channels
                    right = u16;
                    left = u16;
                    allfine = 1;
                } else if (ch == 114) {
                    // 'r' - only change the right channel
                    right = u16;
                    left = *(ptr + (pga * 2 - 2) + 1);
                    allfine = 1;
                } else if (ch == 108) {
                    // 'l' - only change the left channel
                    right = *(ptr + (pga * 2 - 2));
                    left = u16;
                    allfine = 1;
                }
                if (allfine) {
                    pga_set_volume(pga, right, left, 1, 1);
                    uart1_tx_str("ok\r\n", 4);
                }
            }
        }
    }
    //sprintf(str_temp, "D 0x%x 0x%x 0x%x\r\n", input[0], input[1], input[2]);
    //uart1_tx_str(str_temp, strlen(str_temp));

    uart1_p = 0;
    uart1_rx_enable = 1;
}

void uart1_iface_init(void)
{
    sys_messagebus_register(&uart1_rx_irq, SYS_MSG_UART1_RX);
}

uint8_t str_to_uint16(char *str, uint16_t * out, const uint8_t seek,
                      const uint8_t len, const uint16_t min, const uint16_t max)
{
    uint16_t val = 0, pow = 1;
    uint8_t i;

    // pow() is missing in gcc, so we improvise
    for (i = 0; i < len - 1; i++) {
        pow *= 10;
    }
    for (i = 0; i < len; i++) {
        if ((str[seek + i] > 47) && (str[seek + i] < 58)) {
            val += (str[seek + i] - 48) * pow;
        }
        pow /= 10;
    }
    if ((val >= min) && (val <= max)) {
        *out = val;
        return 1;
    } else {
        return 0;
    }
}

#endif

