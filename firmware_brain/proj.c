
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
#include "drivers/uart0.h"
#include "drivers/ir_remote.h"
#include "drivers/pga2311_helper.h"
#include "drivers/lm4780_helper.h"
#include "drivers/flash.h"
#include "drivers/port.h"
#include "ui.h"

#ifdef USE_UART
#include "interface/uart_fcts.h"
#endif

#ifdef USE_I2C
#include "interface/i2c_fcts.h"
  #ifdef HARDWARE_I2C
    #include "drivers/i2c.h"
  #else
    #include "serial_bitbang.h"
  #endif
#endif

int8_t ir_number;
uint8_t pga_id_cur = -1;

#define SLOW_REFRESH_DELAY 56   // 56 ta0 overflows are 56*128s ~= 2hours
uint16_t tfr = SLOW_REFRESH_DELAY;   // time for display refresh

uint8_t d;
char muted[] = "muted";

void display_mixer_status(void)
{
    d = 0;
    tfr = timer_a0_ovf + SLOW_REFRESH_DELAY; // refresh once an hour
    timer_a0_delay_noblk_ccr1(_100ms);
}

static void timer_a0_ovf_irq(enum sys_message msg)
{
    if (timer_a0_ovf >= tfr) {
        if (timer_a0_ovf > 65535 - SLOW_REFRESH_DELAY) {
            return;
        }
        get_mixer_status();
        display_mixer_status();
    }
}

static void timer_a0_ccr1_irq(enum sys_message msg)
{
    switch (d) {
        case 0:
            snprintf(str_temp, TEMP_LEN, "1front     %3d %3d %s\n", s.v[0], s.v[1], mixer_get_mute_struct(1)?"":muted);
            break;
        case 1:
            snprintf(str_temp, TEMP_LEN, "2rear      %3d %3d %s\n", s.v[2], s.v[3], mixer_get_mute_struct(2)?"":muted);
            break;
        case 2:
            snprintf(str_temp, TEMP_LEN, "3line in   %3d %3d %s\n", s.v[4], s.v[5], mixer_get_mute_struct(3)?"":muted);
            break;
        case 3:
            snprintf(str_temp, TEMP_LEN, "4spdif     %3d %3d %s\n", s.v[6], s.v[7], mixer_get_mute_struct(4)?"":muted);
            break;
        case 4:
            snprintf(str_temp, TEMP_LEN, "5f-r pan   %3d %3d %s\n", s.v[8], s.v[9], mixer_get_mute_struct(5)?"":muted);
            break;
        case 5:
            snprintf(str_temp, TEMP_LEN, "6center    %3d     %s\n", s.v[10], mixer_get_mute_struct(6)?"":muted);
            break;
        case 6:
            snprintf(str_temp, TEMP_LEN, "7subwoofer %3d     %s\n", s.v[11], mixer_get_mute_struct(6)?"":muted);
            break;
        case 7:
            snprintf(str_temp, TEMP_LEN, "A\n");
            break;
        default:
            return;
    }

    d++;
    uart0_tx_str(str_temp, strlen(str_temp));
    //timer_a0_delay_noblk_ccr1(_500ms);
    timer_a0_delay_noblk_ccr1(_10ms);
}

static void parse_UI(enum sys_message msg)
{
    parse_user_input();

    uart0_p = 0;
    uart0_rx_enable = 1;
}

// edge detect based interrupt handler
static void port_trigger(enum sys_message msg)
{
    uint8_t in_now[DETECT_CHANNELS] = {0,0};
    uint8_t i;

    if (input_ed & SND_DETECT_FRONT) {
        in_now[0] = MUTE;
    }
    if (input_ed & SND_DETECT_REAR) {
        in_now[1] = MUTE;
    }

    for (i=0;i<DETECT_CHANNELS;i++) {
        if (in_now[i]!=stat.in_orig[i]) {
            stat.count[i]=0;
        }
    }

    timer_a0_delay_noblk_ccr2(_50ms);
}

// time based interrupt request handler
static void port_parser(enum sys_message msg)
{
    uint8_t in_now[DETECT_CHANNELS] = {LIVE, LIVE};
    uint8_t i;
    uint8_t smth_changed = 0;

    if (P1IN & SND_DETECT_FRONT) {
        in_now[0] = MUTE;
    }
    if (P1IN & SND_DETECT_REAR) {
        in_now[1] = MUTE;
    }
           
    for (i=0;i<DETECT_CHANNELS;i++) {
        if (in_now[i]!=stat.in_orig[i]) {
            smth_changed = 1;
            stat.count[i]++;
            if ((stat.in_orig[i] == MUTE) && (stat.count[i] > ON_DEBOUNCE)) {
                stat.count[i] = 0;
                ampy_set_status(i+1, UNMUTE);
                stat.in_orig[i] = UNMUTE;
                LED_ON;
                if (i == 0) {
                    UNMUTE_FRONT;
                } else if (i == 1) {
                    UNMUTE_REAR;
                }
            } else if ((stat.in_orig[i] == UNMUTE) && (stat.count[i] > OFF_DEBOUNCE)) {
                stat.count[i] = 0;
                ampy_set_status(i+1, MUTE);
                stat.in_orig[i] = MUTE;
                if ((ampy_get_status(1) == MUTE) && (ampy_get_status(2) == MUTE)) {
                    LED_OFF;
                }
                if (i == 0) {
                    MUTE_FRONT;
                } else if (i == 1) {
                    MUTE_REAR;
                }
            }
        }
    }

    if (smth_changed) {
        timer_a0_delay_noblk_ccr2(_50ms);
    } else {
        timer_a0_delay_noblk_ccr2(_1s);
    }
}

int main(void)
{
    uint8_t i;

    main_init();
    timer_a0_init();
    ir_init();
    uart0_init();

#ifdef HARDWARE_I2C
    i2c_init();
#endif
    port_init();

    settings_init(FLASH_ADDR);

    for (i=0;i<DETECT_CHANNELS;i++) {
        ampy_set_status(i+1, MUTE);
        stat.in_orig[i] = MUTE;
    }

    sys_messagebus_register(&timer_a0_ovf_irq, SYS_MSG_TIMER0_IFG);
    sys_messagebus_register(&timer_a0_ccr1_irq, SYS_MSG_TIMER0_CCR1);
    sys_messagebus_register(&port_parser, SYS_MSG_TIMER0_CCR2);
    sys_messagebus_register(&port_trigger, SYS_MSG_PORT_TRIG);
    sys_messagebus_register(&parse_UI, SYS_MSG_UART0_RX);

    get_mixer_status();

    while (1) {
        _BIS_SR(LPM3_bits + GIE);
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
    P1DIR = 0x1;
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

    // port mappings
    PMAPPWD = 0x02D52;
    // hardware UART
    P1MAP5 = PM_UCA0TXD;
    P1MAP6 = PM_UCA0RXD;
#ifdef USE_I2C
  #ifdef HARDWARE_I2C
    // set up i2c port mapping
    P1MAP2 = PM_UCB0SCL;
    P1MAP3 = PM_UCB0SDA;
    P1SEL |= BIT2 + BIT3;
  #endif
#endif
    PMAPPWD = 0;

    P1SEL |= BIT5 + BIT6;

}

void check_events(void)
{
    struct sys_messagebus *p = messagebus;
    enum sys_message msg = 0;

    // drivers/timer_a0
    if (timer_a0_last_event) {
        msg |= timer_a0_last_event;
        timer_a0_last_event = 0;
    }
    // drivers/timer1a
    if (timer_a1_last_event) {
        msg |= timer_a1_last_event << 7;
        timer_a1_last_event = 0;
    }
    // drivers/uart0
    if (uart0_last_event == UART0_EV_RX) {
        msg |= SYS_MSG_UART0_RX;
        uart0_last_event = 0;
    }
    // drivers/port
    if (port_last_event) {
        msg |= SYS_MSG_PORT_TRIG;
        port_last_event = 0;
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
            tfr = timer_a0_ovf + 1;
            break;
        case 16:
        case 0x490:            // vol+
            mixer_send_funct(pga_id_cur, FCT_V_INC, VOL_STEP, VOL_STEP);
            tfr = timer_a0_ovf + 1;
            break;
        case 17:
        case 0xc90:            // vol-
            mixer_send_funct(pga_id_cur, FCT_V_DEC, VOL_STEP, VOL_STEP);
            tfr = timer_a0_ovf + 1;
            break;
        case 28:
        case 0x90:             // ch+
            mixer_send_funct(pga_id_cur, FCT_V_INC, VOL_BIG_STEP, VOL_BIG_STEP);
            tfr = timer_a0_ovf + 1;
            break;
        case 29:
        case 0x890:            // ch-
            mixer_send_funct(pga_id_cur, FCT_V_DEC, VOL_BIG_STEP, VOL_BIG_STEP);
            tfr = timer_a0_ovf + 1;
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
            tfr = timer_a0_ovf + 1;
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
        //uart0_tx_str(str_temp, strlen(str_temp));

        ir_resume();            // Receive the next value
    }
}

/*
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
        uart0_tx_str(str_temp, strlen(str_temp));
        return 0;
    }
    return 1;
}
*/

void settings_init(uint8_t * addr)
{
    uint8_t *src_p, *dst_p;
    //uint8_t *ptr;
    //uint8_t right, left;
    uint8_t i;

    src_p = addr;
    dst_p = (uint8_t *) & a;
    if ((*src_p) != A_VER) {
        src_p = (uint8_t *) & defaults;
    }
    for (i = 0; i < sizeof(a); i++) {
        *dst_p++ = *src_p++;
    }
}

void settings_apply(void)
{

}
