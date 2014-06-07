
//  audio mixer based on an MSP430F5510
//
//  author:          Petre Rodan <petre.rodan@simplex.ro>
//  available from:  https://github.com/rodan/ampy
//  license:         GNU GPLv3

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "proj.h"
#include "drivers/sys_messagebus.h"
#include "drivers/timer_a0.h"
#include "drivers/uart1.h"
#include "drivers/flash.h"
#include "drivers/spi.h"

struct settings_t s;

char str_temp[120];

void display_help(void)
{
    sprintf(str_temp,
            "\r\n --- mixer controller ver %d - P1OUT: 0x%x P2OUT: 0x%x P4OUT: 0x%x\r\n",
            VERSION, *(uint8_t *) CS_OUT[5], *(uint8_t *) CS_OUT[3],
            *(uint8_t *) CS_OUT[0]);
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

    ptr = &s.ver;

    for (i = 0; i < 14; i++) {
        sprintf(str_temp, "m%d\r\n", *ptr++);
        uart1_tx_str(str_temp, strlen(str_temp));
    }
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

void mute_pga(const uint8_t pga_id, const uint8_t save)
{
    uint8_t *ptr;

    // set the proper PxOUT port to low in order to mute the PGA out
    ptr = (uint8_t *) MUTE_OUT[pga_id - 1];
    *ptr &= ~MUTE_PORT[pga_id - 1];
    if (save) {
        s.mute_flag &= ~(1 << (pga_id - 1));
    }
}

void unmute_pga(const uint8_t pga_id, const uint8_t save)
{
    uint8_t *ptr;

    // unmute port
    ptr = (uint8_t *) MUTE_OUT[pga_id - 1];
    *ptr |= MUTE_PORT[pga_id - 1];
    if (save) {
        // save values into the settings structure
        s.mute_flag |= 1 << (pga_id - 1);
    }
}

void set_volume(const uint8_t pga_id, const uint8_t vol_right,
                const uint8_t vol_left, const uint8_t save,
                const uint8_t autounmute)
{
    uint8_t data[2];
    uint8_t *ptr;

    led_on;
    if ((vol_left == 0) && (vol_right == 0)) {
        mute_pga(pga_id, save);
    } else {
        data[0] = vol_right;
        data[1] = vol_left;

        if (autounmute) {
            unmute_pga(pga_id, save);
        }

        if (save) {
            // save values into the settings structure
            ptr = (uint8_t *) & s.v1_r;
            *(ptr + (pga_id * 2 - 2)) = vol_right;
            *(ptr + (pga_id * 2 - 2) + 1) = vol_left;
        }
        // select slave
        ptr = (uint8_t *) CS_OUT[pga_id - 1];
        *ptr &= ~CS_PORT[pga_id - 1];

        // set volume
        spi_send_frame(data, 2);

        // deselect slave
        *ptr |= CS_PORT[pga_id - 1];
    }
    led_off;
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

    if ((p == 63) || (p == 72)) {       // [h?]
        display_help();
    } else if (p == 83) {       // [s]how settings
        show_settings();
    } else if (p == 87) {       // [w]rite to flash
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
    } else if (p == 82) {       // [r]ead from flash
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
    } else if (p == 77) {       // [m]ute pga
        if (str_to_uint16(input, &u16, 1, 1, 1, 6)) {
            pga = u16;
            mute_pga(pga, 1);
            uart1_tx_str("ok\r\n", 4);
        }
    } else if (p == 85) {       // [u]nmute pga
        if (str_to_uint16(input, &u16, 1, 1, 1, 6)) {
            pga = u16;
            unmute_pga(pga, 1);
            uart1_tx_str("ok\r\n", 4);
        }
    } else if (p == 86) {       // [v]olume set
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
                    set_volume(pga, right, left, 1, 1);
                    uart1_tx_str("ok\r\n", 4);
                }
            }
        }
    }
    //sprintf(str_temp, "%d\r\n", p);
    //uart1_tx_str(str_temp, strlen(str_temp));

    uart1_p = 0;
    uart1_rx_enable = 1;
}

int main(void)
{
    main_init();
    uart1_init();
    settings_init(FLASH_ADDR);

    sys_messagebus_register(&uart1_rx_irq, SYS_MSG_UART1_RX);
    led_off;

    while (1) {
        // sleep
        _BIS_SR(LPM3_bits + GIE);
        __no_operation();
        //wake_up();
#ifdef USE_WATCHDOG
        WDTCTL = (WDTCTL & 0xff) | WDTPW | WDTCNTCL;
#endif
        check_events();
    }
}

void main_init(void)
{
    // watchdog triggers after 25sec when not cleared
#ifdef USE_WATCHDOG
    WDTCTL = WDTPW + WDTIS__512K + WDTSSEL__ACLK + WDTCNTCL;
#else
    WDTCTL = WDTPW + WDTHOLD;
#endif

    P1SEL = 0;
    P1DIR = 0xff;
    P1OUT = 0;

    P2SEL = 0;
    P2DIR = 0x1;
    P2OUT = 0;

    P3SEL = 0;
    P3DIR = 0x1f;
    P3OUT = 0;

    P4SEL = 0x0a;
    P4DIR = 0xff;
    P4OUT = 0;

    //P5SEL is set above
    P5SEL = 0x30;
    P5DIR = 0xff;
    P5OUT = 0;

    P6DIR = 0xff;
    P6OUT = 0x1;

    PJDIR = 0xff;
    PJOUT = 0;

    // disable VUSB LDO and SLDO
    USBKEYPID = 0x9628;
    USBPWRCTL &= ~(SLDOEN + VUSBEN);
    USBKEYPID = 0x9600;

    timer_a0_init();

    // PGA2311 needs to get the digital power after a delay
    // otherwise it will lock up
    timer_a0_delay(1000000);
    pga_enable;
    spi_init();
    spi_fast_mode();

    // set chip selects high (deselect all slaves)
    P1OUT |= 0x54;
    P2OUT |= 0x1;
    P4OUT |= 0x84;
}

void settings_init(uint8_t * addr)
{
    uint8_t *src_p, *dst_p;
    uint8_t *ptr;
    uint8_t right, left;
    uint8_t i;

    src_p = addr;
    dst_p = (uint8_t *) & s;
    if ((*src_p) != VERSION) {
        src_p = (uint8_t *) & defaults;
    }
    for (i = 0; i < sizeof(s); i++) {
        *dst_p++ = *src_p++;
    }

    // apply settings to hardware

    ptr = (uint8_t *) & s.v1_r;
    for (i = 1; i < 7; i++) {
        right = *(ptr + (i * 2 - 2));
        left = *(ptr + (i * 2 - 2) + 1);
        if ((left != 0) && (right != 0)) {
            set_volume(i, right, left, 0, 0);
        }
        if (s.mute_flag & (1 << (i - 1))) {
            unmute_pga(i, 0);
        }
    }

}

/*
void wake_up(void)
{
}
*/

void check_events(void)
{
    struct sys_messagebus *p = messagebus;
    enum sys_message msg = 0;

    // drivers/timer_a0
    if (timer_a0_last_event == TIMER_A0_EVENT_IFG) {
        msg |= BIT0;
        timer_a0_last_event = 0;
    }
    // uart RX
    if (uart1_last_event == UART1_EV_RX) {
        msg |= BIT2;
        uart1_last_event = 0;
    }

    while (p) {
        // notify listener if he registered for any of these messages
        if (msg & p->listens) {
            p->fn(msg);
        }
        p = p->next;
    }
}
