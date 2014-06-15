
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
#include "drivers/ir_remote.h"
#include "drivers/pga2311.h"

char str_temp[64];

uint8_t idx;

int8_t ir_number, pga_id = -1;

void save_presets(uint8_t location)
{
    sprintf(str_temp, "w%d\n", location);
    uart_tx_str(str_temp, strlen(str_temp));
}

void load_presets(uint8_t location)
{
    sprintf(str_temp, "r%d\n", location);
    uart_tx_str(str_temp, strlen(str_temp));
}

void get_mixer_status(void)
{
    idx = 0;
    //timer_a1_halt();
    uart_tx_str("s\n", 3);
}

void display_mixer_status(void)
{
    char m[] = "muted";
    //sprintf(str_temp, "p1dir:%x p1sel:%x\n", P1DIR, P1SEL);
    //uart_tx_str(str_temp, strlen(str_temp));

    sprintf(str_temp, "1front     %3d %3d %s\n", s.v1_r, s.v1_l, (s.mute_flag & 1)?"":m);
    uart_tx_str(str_temp, strlen(str_temp));
    timer_a0_delay(600000);

    sprintf(str_temp, "2rear      %3d %3d %s\n", s.v2_r, s.v2_l, (s.mute_flag & (1 << 1))?"":m);
    uart_tx_str(str_temp, strlen(str_temp));
    timer_a0_delay(600000);

    sprintf(str_temp, "3line in   %3d %3d %s\n", s.v3_r, s.v3_l, (s.mute_flag & (1 << 2))?"":m);
    uart_tx_str(str_temp, strlen(str_temp));
    timer_a0_delay(600000);

    sprintf(str_temp, "4spdif     %3d %3d %s\n", s.v4_r, s.v4_l, (s.mute_flag & (1 << 3))?"":m);
    uart_tx_str(str_temp, strlen(str_temp));
    timer_a0_delay(600000);

    sprintf(str_temp, "5f-r pan   %3d %3d %s\n", s.v5_r, s.v5_l, (s.mute_flag & (1 << 4))?"":m);
    uart_tx_str(str_temp, strlen(str_temp));
    timer_a0_delay(600000);

    sprintf(str_temp, "6center    %3d     %s\n", s.v6_r, (s.mute_flag & (1 << 5))?"":m);
    uart_tx_str(str_temp, strlen(str_temp));
    timer_a0_delay(600000);

    sprintf(str_temp, "7subwoofer %3d     %s\n", s.v6_l, (s.mute_flag & (1 << 5))?"":m);
    uart_tx_str(str_temp, strlen(str_temp));
    timer_a0_delay(600000);

    uart_tx_str("A\n", 3);
}

static void uart_rx_irq(enum sys_message msg)
{
    uint16_t u16;
    uint8_t p, i;
    char *input;
    uint8_t *src, *dst;

    input = (char *)uart_rx_buf;

    p = input[0];
    if (p >= 97) {
        p -= 32;
    }

    if (p == 77) {               // m - volume values
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
    } else if (p == 79) {         // ok - ACK 

    }

    //sprintf(str_temp, "\r\n%d\r\n", p);
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

    sys_messagebus_register(&uart_rx_irq, SYS_MSG_UART_RX);

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
    //uint16_t timeout = 5000;

    // watchdog triggers after 4 minutes when not cleared
#ifdef USE_WATCHDOG
    WDTCTL = WDTPW + WDTIS__8192K + WDTSSEL__ACLK + WDTCNTCL;
#else
    WDTCTL = WDTPW + WDTHOLD;
#endif
    SetVCore(3);

    P1SEL = 0x0;
    P1DIR = 0xcf;
    P1OUT = 0x0;

    P2SEL = 0x0;
    P2DIR = 0xff;
    P2OUT = 0x0;

    P3SEL = 0x0;
    P3DIR = 0xff;
    P3OUT = 0x0;

    P5SEL |= BIT0 + BIT1;

    PJDIR = 0xFF;
    PJOUT = 0x00;

    /*
    // send MCLK to P1.2
    __disable_interrupt();
    // get write-access to port mapping registers
    //PMAPPWD = 0x02D52;
    PMAPPWD = PMAPKEY;
    PMAPCTL = PMAPRECFG;
    // MCLK set out to 1.2
    P1MAP2 = PM_MCLK;
    //P1MAP2 = PM_RTCCLK;
    PMAPPWD = 0;
    __enable_interrupt();
    P1DIR |= BIT2;
    P1SEL |= BIT2;

    // send ACLK to P1.0
    //P1DIR |= BIT0;
    //P1SEL |= BIT0;
    */
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

#ifdef USE_UART_TO_MIXER

void mixer_send_funct(const uint8_t pga, const uint8_t function, const uint8_t r_diff, const uint8_t l_diff) 
{
    uint8_t vol_r, vol_l;

    switch (function) {

    case FCT_T_MUTE:
        if (!mixer_get_mute_struct(pga)) {
            mixer_set_mute_struct(pga, UNMUTE);
            sprintf(str_temp, "u%d\n", pga);
        } else {
            mixer_set_mute_struct(pga, MUTE);
            sprintf(str_temp, "m%d\n", pga);
        }
        uart_tx_str(str_temp, strlen(str_temp));
    break;

    case FCT_V_INC:
        vol_r = mixer_get_vol_struct(pga, CH_RIGHT);
        if (vol_r < 255 - r_diff) {
            vol_r+=r_diff;
            mixer_set_vol_struct(pga, CH_RIGHT, vol_r);
        }
        vol_l = mixer_get_vol_struct(pga, CH_LEFT);
        if (vol_l < 255 - l_diff) {
            vol_l+=l_diff;
            mixer_set_vol_struct(pga, CH_LEFT, vol_l);
        }
        if (vol_r == vol_l) {
            sprintf(str_temp, "v%db%d\n", pga, vol_r);
            uart_tx_str(str_temp, strlen(str_temp));
        } else {
            sprintf(str_temp, "v%dr%d\n", pga, vol_r);
            uart_tx_str(str_temp, strlen(str_temp));

            sprintf(str_temp, "v%dl%d\n", pga, vol_l);
            uart_tx_str(str_temp, strlen(str_temp));
        }
    break;

    case FCT_V_DEC:
        vol_r = mixer_get_vol_struct(pga, CH_RIGHT);
        if (vol_r > r_diff) {
            vol_r-=r_diff;
            mixer_set_vol_struct(pga, CH_RIGHT, vol_r);
        }
        vol_l = mixer_get_vol_struct(pga, CH_LEFT);
        if (vol_l > l_diff) {
            vol_l-=l_diff;
            mixer_set_vol_struct(pga, CH_LEFT, vol_l);
        }
        if (vol_r == vol_l) {
            sprintf(str_temp, "v%db%d\n", pga, vol_r);
            uart_tx_str(str_temp, strlen(str_temp));
        } else {
            sprintf(str_temp, "v%dr%d\n", pga, vol_r);
            uart_tx_str(str_temp, strlen(str_temp));

            sprintf(str_temp, "v%dl%d\n", pga, vol_l);
            uart_tx_str(str_temp, strlen(str_temp));
        }
    break;
    }
}
#else

void mixer_send_funct(const uint8_t pga, const uint8_t function, const uint8_t r_diff, const uint8_t l_diff) 
{
    uint8_t vol_r, vol_l;

    switch (function) {

    case FCT_T_MUTE:
        if (!mixer_get_mute_struct(pga)) {
            mixer_set_mute_struct(pga, UNMUTE);
            //sprintf(str_temp, "u%d\n", pga);
        } else {
            mixer_set_mute_struct(pga, MUTE);
            //sprintf(str_temp, "m%d\n", pga);
        }
        //uart_tx_str(str_temp, strlen(str_temp));
    break;

    case FCT_V_INC:
        vol_r = mixer_get_vol_struct(pga, CH_RIGHT);
        if (vol_r < 255 - r_diff) {
            vol_r+=r_diff;
            mixer_set_vol_struct(pga, CH_RIGHT, vol_r);
        }
        vol_l = mixer_get_vol_struct(pga, CH_LEFT);
        if (vol_l < 255 - l_diff) {
            vol_l+=l_diff;
            mixer_set_vol_struct(pga, CH_LEFT, vol_l);
        }
        if (vol_r == vol_l) {
            //sprintf(str_temp, "v%db%d\n", pga, vol_r);
            //uart_tx_str(str_temp, strlen(str_temp));
        } else {
            //sprintf(str_temp, "v%dr%d\n", pga, vol_r);
            //uart_tx_str(str_temp, strlen(str_temp));

            //sprintf(str_temp, "v%dl%d\n", pga, vol_l);
            //uart_tx_str(str_temp, strlen(str_temp));
        }
    break;

    case FCT_V_DEC:
        vol_r = mixer_get_vol_struct(pga, CH_RIGHT);
        if (vol_r > r_diff) {
            vol_r-=r_diff;
            mixer_set_vol_struct(pga, CH_RIGHT, vol_r);
        }
        vol_l = mixer_get_vol_struct(pga, CH_LEFT);
        if (vol_l > l_diff) {
            vol_l-=l_diff;
            mixer_set_vol_struct(pga, CH_LEFT, vol_l);
        }
        if (vol_r == vol_l) {
            //sprintf(str_temp, "v%db%d\n", pga, vol_r);
            //uart_tx_str(str_temp, strlen(str_temp));
        } else {
            //sprintf(str_temp, "v%dr%d\n", pga, vol_r);
            //uart_tx_str(str_temp, strlen(str_temp));

            //sprintf(str_temp, "v%dl%d\n", pga, vol_l);
            //uart_tx_str(str_temp, strlen(str_temp));
        }
    break;
    }
}

#endif

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
            mixer_send_funct(pga_id, FCT_T_MUTE, 0, 0);
            break;
        case 16:
        case 0x490:            // vol+
            mixer_send_funct(pga_id, FCT_V_INC, VOL_STEP, VOL_STEP);
            break;
        case 17:
        case 0xc90:            // vol-
            mixer_send_funct(pga_id, FCT_V_DEC, VOL_STEP, VOL_STEP);
            break;
        case 28:
/*
        case 0x90:             // ch+
            break;
        case 29:
        case 0x890:            // ch-
            break;
*/
        case 36:               // record
            save_presets(1); 
            break;
/*
        case 54:
        case 0xa90:            // stop
            break;
*/
        case 14:               // play
            load_presets(1);
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

        if (ir_number > -1) {
            pga_id = ir_number;
        }

        //sprintf(str_temp, "%ld\r\n", results.value);
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
        sprintf(str_temp, "\e[31;1merr\e[0m specify an int between %u-%u\r\n",
                min, max);
        uart_tx_str(str_temp, strlen(str_temp));
        return 0;
    }
    return 1;
}


