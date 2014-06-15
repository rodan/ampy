#ifndef __PGA2311_CONFIG_H__
#define __PGA2311_CONFIG_H__

#include "proj.h"

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

#endif
