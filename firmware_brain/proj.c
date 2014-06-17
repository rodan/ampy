
//
//  author:          Petre Rodan <petre.rodan@simplex.ro>
//  available from:  https://github.com/rodan/ampy
//  license:         GNU GPLv3

#include <stdio.h>
#include <string.h>

#include "proj.h"
#include "drivers/sys_messagebus.h"
#include "drivers/pmm.h"
#include "drivers/timer_a0.h"
#include "drivers/timer_a1.h"
#include "drivers/uart.h"
#include "drivers/i2c.h"
#include "drivers/ir_remote.h"
#include "drivers/pga2311_helper.h"

#ifdef USE_UART
#include "interface/uart_fcts.h"
#endif

#ifdef USE_I2C
#include "interface/i2c_fcts.h"
#endif

int8_t ir_number;
uint8_t pga_id_cur = -1;

#define DISPLAY_DELAY 400000

void display_mixer_status(void)
{

    char m[] = "muted";

    snprintf(str_temp, TEMP_LEN, "1front     %3d %3d %s\n", s.v1_r, s.v1_l, mixer_get_mute_struct(1)?"":m);
    uart_tx_str(str_temp, strlen(str_temp));
    timer_a0_delay(DISPLAY_DELAY);

    snprintf(str_temp, TEMP_LEN, "2rear      %3d %3d %s\n", s.v2_r, s.v2_l, mixer_get_mute_struct(2)?"":m);
    uart_tx_str(str_temp, strlen(str_temp));
    timer_a0_delay(DISPLAY_DELAY);

    snprintf(str_temp, TEMP_LEN, "3line in   %3d %3d %s\n", s.v3_r, s.v3_l, mixer_get_mute_struct(3)?"":m);
    uart_tx_str(str_temp, strlen(str_temp));
    timer_a0_delay(DISPLAY_DELAY);

    snprintf(str_temp, TEMP_LEN, "4spdif     %3d %3d %s\n", s.v4_r, s.v4_l, mixer_get_mute_struct(4)?"":m);
    uart_tx_str(str_temp, strlen(str_temp));
    timer_a0_delay(DISPLAY_DELAY);

    snprintf(str_temp, TEMP_LEN, "5f-r pan   %3d %3d %s\n", s.v5_r, s.v5_l, mixer_get_mute_struct(5)?"":m);
    uart_tx_str(str_temp, strlen(str_temp));
    timer_a0_delay(DISPLAY_DELAY);

    snprintf(str_temp, TEMP_LEN, "6center    %3d     %s\n", s.v6_r, mixer_get_mute_struct(6)?"":m);
    uart_tx_str(str_temp, strlen(str_temp));
    timer_a0_delay(DISPLAY_DELAY);

    snprintf(str_temp, TEMP_LEN, "7subwoofer %3d     %s\n", s.v6_l, mixer_get_mute_struct(6)?"":m);
    uart_tx_str(str_temp, strlen(str_temp));
    timer_a0_delay(DISPLAY_DELAY);

    uart_tx_str("A\n", 3);
    timer_a0_delay(DISPLAY_DELAY);
}

static void uart_rx_irq(enum sys_message msg)
{
    //uint16_t u16;
    uint8_t p;
    //uint8_t i;
    char *input;
    //uint8_t *src, *dst;

    input = (char *)uart_rx_buf;

    p = input[0];
    if (p >= 97) {
        p -= 32;
    }

    if (p == 77) {               // m - volume values
        /*
        if (str_to_uint16(input, &u16, 1, strlen(input) - 1, 0, 255)) {
            src = (uint8_t *) & s;
            *(src+idx) = u16;
            if (idx > 12) {
                idx = 0;
                for (i=0;i<14;i++) {
                    dst = (uint8_t *) & s;
                    *(dst + i) = *(src + i);
                }
                //timer_a1_init();
                display_mixer_status();
            }
            idx++;
        }
        */
    } else if (p == 79) {         // ok - ACK 

    }

    //snprintf(str_temp, TEMP_LEN, "\r\n%d\r\n", p);
    //uart_tx_str(str_temp, strlen(str_temp));

    uart_p = 0;
    uart_rx_enable = 1;
}

int main(void)
{
    main_init();
    timer_a0_init();
    ir_init();
    uart_init();

#ifdef USE_I2C
    // set up i2c port mapping
    PMAPPWD = 0x02D52;
    P1MAP2 = PM_UCB0SCL;
    P1MAP3 = PM_UCB0SDA;
    PMAPPWD = 0;
    P1SEL |= BIT2 + BIT3;

    i2c_init();
#endif

    sys_messagebus_register(&uart_rx_irq, SYS_MSG_UART_RX);

    // PGAs are started up with a delay, so wait a little
    // before querying them
    timer_a0_delay(1000000);
    timer_a0_delay(1000000);
    timer_a0_delay(1000000);
    get_mixer_status();

    while (1) {
        //sleep();
        //__no_operation();
        //wake_up();
#ifdef USE_WATCHDOG
        WDTCTL = (WDTCTL & 0xff) | WDTPW | WDTCNTCL;
#endif
        check_events();
        check_ir();
    }
}

void main_init(void)
{

    // watchdog triggers after 4 minutes when not cleared
#ifdef USE_WATCHDOG
    WDTCTL = WDTPW + WDTIS__8192K + WDTSSEL__ACLK + WDTCNTCL;
#else
    WDTCTL = WDTPW + WDTHOLD;
#endif
    SetVCore(3);

    P1SEL = 0x0;
    P1DIR = 0x43;
    P1OUT = 0x0;

    P2SEL = 0x0;
    P2DIR = 0xff;
    P2OUT = 0x0;

    P3SEL = 0x0;
    P3DIR = 0xff;
    P3OUT = 0x0;

    P5SEL |= BIT0 + BIT1;

    PJDIR = 0xff;
    PJOUT = 0x00;

}

void sleep(void)
{
    _BIS_SR(LPM3_bits + GIE);
    __no_operation();
}

void wake_up(void)
{

}

void check_events(void)
{
    struct sys_messagebus *p = messagebus;
    enum sys_message msg = 0;

    // drivers/timer1a
    if (timer_a1_last_event) {
        msg |= timer_a1_last_event << 7;
        timer_a1_last_event = 0;
    }
    // drivers/uart
    if (uart_last_event == UART_EV_RX) {
        msg |= BITA;
        uart_last_event = 0;
    }
    while (p) {
        // notify listener if he registered for any of these messages
        if (msg & p->listens) {
            p->fn(msg);
        }
        p = p->next;
    }
}

void check_ir(void)
{
    if (ir_decode(&results)) {

        ir_number = -1;

        switch (results.value) {
            // RC5 codes
        case 1:                // 1
            ir_number = 1;
            break;
        case 2:                // 2
            ir_number = 2;
            break;
        case 3:                // 3
            ir_number = 3;
            break;
        case 4:                // 4
            ir_number = 4;
            break;
        case 5:                // 5
            ir_number = 5;
            break;
        case 6:                // 6
            ir_number = 6;
            break;
        case 7:                // 7
            ir_number = 7;
            break;
        case 8:                // 8
            ir_number = 8;
            break;
        case 9:                // 9
            ir_number = 9;
            break;
        case 0:                // 0
            ir_number = 0;
            break;
/*
        case 10: // 10
            ir_number = 10;
            break;
        case 12:               // power
            // wake up from pwr_down
            break;
        case 56:               // AV
            in_number = 0;
            break;
        case 36: // red
          break;
        case 35: // green
          break;
        case 14: // yellow
          break;
        case 50: // zoom
            break;
*/
        case 39: // sub
            display_mixer_status();
            break;
/*
        case 44: // slow
            break;
*/
        case 60: // repeat
            get_mixer_status();
            break;
/*
        case 15: // disp
            break;
        case 38: // sleep
            break;
        case 32: // up
            break;
        case 33: // down
            break;
        case 16: // right
            break;
        case 17: // left
            break;
        case 59: // ok
            break;
        case 34: // back
            break;
        case 19: // exit
            break;
        case 18: // menu
            break;
*/
        case 13:
        case 0x290:            // mute
            mixer_send_funct(pga_id_cur, FCT_T_MUTE, 0, 0);
            break;
        case 16:
        case 0x490:            // vol+
            mixer_send_funct(pga_id_cur, FCT_V_INC, VOL_STEP, VOL_STEP);
            break;
        case 17:
        case 0xc90:            // vol-
            mixer_send_funct(pga_id_cur, FCT_V_DEC, VOL_STEP, VOL_STEP);
            break;
        case 28:
        case 0x90:             // ch+
            mixer_send_funct(pga_id_cur, FCT_V_INC, VOL_BIG_STEP, VOL_BIG_STEP);
            break;
        case 29:
        case 0x890:            // ch-
            mixer_send_funct(pga_id_cur, FCT_V_DEC, VOL_BIG_STEP, VOL_BIG_STEP);
            break;
        case 36:               // record
            save_presets(pga_id_cur); 
            break;
/*
        case 54:
        case 0xa90:            // stop
            break;
*/
        case 14:               // play
            load_presets(pga_id_cur);
            break;
/*
        case 31:               // pause
            break;
        case 35:
        case 0xa50:            // rew, AV/TV
            break;
        case : // fwd
            break;
*/
        default:
            break;
        }                       // switch

        if (ir_number > 0) {
            pga_id_cur = ir_number;
        }

        //snprintf(str_temp, TEMP_LEN, "%ld\r\n", results.value);
        //uart_tx_str(str_temp, strlen(str_temp));
        ir_resume();            // Receive the next value
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
    } else {
        snprintf(str_temp, TEMP_LEN, "\e[31;1merr\e[0m specify an int between %u-%u\r\n",
                min, max);
        uart_tx_str(str_temp, strlen(str_temp));
        return 0;
    }
    return 1;
}



