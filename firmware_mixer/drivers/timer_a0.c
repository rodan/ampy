
//   timer a0 handling
//   CCR0 is currently unused
//   CCR1 is currently unused
//   CCR2 is used for timer_a0_delay_noblk()
//   CCR3 is currently unused
//   CCR4 is used for timer_a0_delay()
//
//   author:          Petre Rodan <2b4eda@subdimension.ro>
//   available from:  https://github.com/rodan/
//   license:         GNU GPLv3

#include "timer_a0.h"

// ticks = microseconds / 30.5175 if no input divider
// ticks = microseconds / 244.14 if ID__8 is used
// ticks = microseconds / 1953.12 if ID__8 and TAIDEX 0x7

void timer_a0_init(void)
{
    __disable_interrupt();
    timer_a0_ovf = 0;

    TA0EX0 |= TAIDEX_7; // divide input clock by 8, and then by 8 again
    TA0CTL |= TASSEL__ACLK + MC__CONTINOUS + TACLR + ID__8 + TAIE;

    // for ID__8 and TAIDEX 0x7
    //   Fclock = 32768/8/8 = 512Hz
    //   1 tick = 1953 microseconds
    //   Tovf   = 65536 * .0019531250 = 128s 
    __enable_interrupt();
}

void timer_a0_delay_ccr4(uint16_t ticks)
{
    __disable_interrupt();
    TA0CCR4 = TA0R + ticks;
    TA0CCTL4 = CCIE;
    __enable_interrupt();
    timer_a0_last_event &= ~TIMER_A0_EVENT_CCR4;
    while (1) {
        _BIS_SR(LPM3_bits + GIE);
        __no_operation();
#ifdef USE_WATCHDOG
        // reset watchdog
        WDTCTL = (WDTCTL & 0xff) | WDTPW | WDTCNTCL;
#endif
        if (timer_a0_last_event & TIMER_A0_EVENT_CCR4)
            break;
    }
    TA0CCTL4 &= ~CCIE;
    timer_a0_last_event &= ~TIMER_A0_EVENT_CCR4;
}

void timer_a0_delay_noblk_ccr1(uint16_t ticks)
{
    TA0CCTL1 &= ~CCIE;
    TA0CCTL1 = 0;
    TA0CCR1 = TA0R + ticks;
    TA0CCTL1 = CCIE;
}

void timer_a0_delay_noblk_ccr2(uint16_t ticks)
{
    TA0CCTL2 &= ~CCIE;
    TA0CCTL2 = 0;
    TA0CCR2 = TA0R + ticks;
    TA0CCTL2 = CCIE;
}

void timer_a0_delay_noblk_ccr3(uint16_t ticks)
{
    TA0CCTL3 &= ~CCIE;
    TA0CCTL3 = 0;
    TA0CCR3 = TA0R + ticks;
    TA0CCTL3 = CCIE;
}

__attribute__ ((interrupt(TIMER0_A1_VECTOR)))
void timer0_A1_ISR(void)
{
    uint16_t iv = TA0IV;
    if (iv == TA0IV_TA0CCR4) {
        // timer used by timer_a0_delay()
        timer_a0_last_event |= TIMER_A0_EVENT_CCR4;
        _BIC_SR_IRQ(LPM3_bits);
    } else if (iv == TA0IV_TA0CCR1) {
        // timer used by timer_a0_delay_noblk_ccr1()
        // disable interrupt
        TA0CCTL1 &= ~CCIE;
        TA0CCTL1 = 0;
        timer_a0_last_event |= TIMER_A0_EVENT_CCR1;
        _BIC_SR_IRQ(LPM3_bits);
    } else if (iv == TA0IV_TA0CCR2) {
        // timer used by timer_a0_delay_noblk_ccr2()
        // disable interrupt
        TA0CCTL2 &= ~CCIE;
        TA0CCTL2 = 0;
        timer_a0_last_event |= TIMER_A0_EVENT_CCR2;
        _BIC_SR_IRQ(LPM3_bits);
    } else if (iv == TA0IV_TA0CCR3) {
        // timer used by timer_a0_delay_noblk_ccr3()
        // disable interrupt
        TA0CCTL3 &= ~CCIE;
        TA0CCTL3 = 0;
        timer_a0_last_event |= TIMER_A0_EVENT_CCR3;
        _BIC_SR_IRQ(LPM3_bits);
    } else if (iv == TA0IV_TA0IFG) {
        TA0CTL &= ~TAIFG;
        timer_a0_ovf++;
        timer_a0_last_event |= TIMER_A0_EVENT_IFG;
        _BIC_SR_IRQ(LPM3_bits);
    }
}
