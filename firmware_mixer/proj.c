
//  audio mixer based on an MSP430F5510
//
//  author:          Petre Rodan <petre.rodan@simplex.ro>
//  available from:  https://github.com/rodan/ampy
//  license:         GNU GPLv3

#include "proj.h"
#include "drivers/sys_messagebus.h"
#include "drivers/timer_a0.h"
#include "drivers/flash.h"
#include "drivers/spi.h"
#include "drivers/pga2311.h"
#include "drivers/pga2311_helper.h"
#include "drivers/adc.h"

#ifdef USE_UART
#include "drivers/uart1.h"
#include "interface/uart_fcts.h"
#endif

#ifdef USE_I2C
#include "drivers/i2c_slave.h"
#include "interface/i2c_fcts.h"
#endif

// once a while get the internal temperature
#define SLOW_REFRESH_DELAY 1   // 1 ta0 overflows are 1*128s ~= 2min
uint16_t tfr = 0;   // get temperature

void get_temperature(void)
{
    uint16_t q_temp;

    adc10_read(10, &q_temp, REFVSEL_0);
    adc10_halt();
    s.int_temp = (int8_t) calc_temp(q_temp);
}

static void timer_a0_ovf_irq(enum sys_message msg)
{
    if (timer_a0_ovf >= tfr) {
        if (timer_a0_ovf > 65535 - SLOW_REFRESH_DELAY) {
            return;
        }
        tfr = timer_a0_ovf + SLOW_REFRESH_DELAY;
        get_temperature();
    }
}

int main(void)
{
    main_init();
    timer_a0_init();

    // PGA2311 needs to get the digital power after a delay
    // otherwise it will lock up
    timer_a0_delay_ccr4(_1s);
    pga_enable;
    spi_init();
    spi_fast_mode();

    // set chip selects high (deselect all slaves)
    P1OUT |= 0x54;
    P2OUT |= 0x1;
    P4OUT |= 0x84;

    settings_init(FLASH_ADDR);

    get_temperature();

#ifdef USE_UART
    uart1_init();
    uart1_iface_init();
#endif

#ifdef USE_I2C
    i2c_slave_init();
    i2c_iface_init();
#endif

    sys_messagebus_register(&timer_a0_ovf_irq, SYS_MSG_TIMER0_IFG);

    led_off;

    while (1) {
        // go into low power mode until an IRQ wakes us up
        _BIS_SR(LPM0_bits + GIE);
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

    P4DIR = 0xcf;
    P4OUT = 0;

    PMAPPWD = 0x02D52;
    P4MAP1 = PM_UCB1SIMO;
    P4MAP3 = PM_UCB1CLK;
#ifdef USE_I2C
    // set up i2c port mapping
    P4MAP4 = PM_UCB0SCL;
    P4MAP5 = PM_UCB0SDA;
    P4SEL |= BIT4 + BIT5;
#endif
    P4SEL |= BIT1 + BIT3;
    PMAPPWD = 0;

    P5SEL = 0x30;
    P5DIR = 0xff;
    P5OUT = 0;

    P6DIR = 0xff;
    P6OUT = 0x1;

    PJDIR = 0xFF;
    PJOUT = 0;

    // disable VUSB LDO and SLDO
    USBKEYPID = 0x9628;
    USBPWRCTL &= ~(SLDOEN + VUSBEN);
    USBKEYPID = 0x9600;
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

    ptr = (uint8_t *) & s.v;
    for (i = 1; i < 7; i++) {
        right = *(ptr + (i * 2 - 2));
        left = *(ptr + (i * 2 - 2) + 1);
        if ((left != 0) && (right != 0)) {
            pga_set_volume(i, right, left, 0, 0);
        }
        if (s.mute_flag & (1 << (i - 1))) {
            pga_set_mute_st(i, UNMUTE, 0);
        } else {
            pga_set_mute_st(i, MUTE, 0);
        }
    }

}

void wake_up(void)
{
}

void check_events(void)
{
    struct sys_messagebus *p = messagebus;
    enum sys_message msg = 0;

    // drivers/timer_a0
    if (timer_a0_last_event == TIMER_A0_EVENT_IFG) {
        msg |= BIT0;
        timer_a0_last_event = 0;
    }

#ifdef USE_UART
    // drivers/uart
    if (uart1_last_event == UART1_EV_RX) {
        msg |= BIT2;
        uart1_last_event = 0;
    }
#endif
#ifdef USE_I2C
    // drivers/i2c_slave
    if (i2c_last_event == I2C_EV_RX) {
        msg |= BIT3;
        i2c_last_event = 0;
    }
#endif

    while (p) {
        // notify listener if he registered for any of these messages
        if (msg & p->listens) {
            p->fn(msg);
        }
        p = p->next;
    }
}

