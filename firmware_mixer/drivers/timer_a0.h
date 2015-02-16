#ifndef __TIMER_A0_H__
#define __TIMER_A0_H__

#include "proj.h"

#define _10ms           5UL       // ~10ms
#define _50ms           _10ms * 5
#define _100ms          _10ms * 10
#define _500ms          _10ms * 50
#define _1s             512UL

enum timer_a0_event {
    TIMER_A0_EVENT_CCR0 = BIT0,
    TIMER_A0_EVENT_CCR1 = BIT1,
    TIMER_A0_EVENT_CCR2 = BIT2,
    TIMER_A0_EVENT_CCR3 = BIT3,
    TIMER_A0_EVENT_CCR4 = BIT4,
    TIMER_A0_EVENT_IFG = BIT5,
};

volatile enum timer_a0_event timer_a0_last_event;
volatile uint16_t timer_a0_ovf;

void timer_a0_init(void);
void timer_a0_halt(void);
void timer_a0_delay_noblk_ccr1(uint16_t ticks);
void timer_a0_delay_noblk_ccr2(uint16_t ticks);
void timer_a0_delay_noblk_ccr3(uint16_t ticks);
void timer_a0_delay_ccr4(uint16_t ticks);

#endif
