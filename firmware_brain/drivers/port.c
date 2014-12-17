
#include <stdint.h>
#include "port.h"
#include "proj.h"

#define ALL_INPUTS  0x82

void port_init(void)
{
	// IRQ triggers on a hi-low transition
	P1IES |= ALL_INPUTS;
	P1IFG &= ~ALL_INPUTS;
	P1IE |= ALL_INPUTS;
}

__attribute__((interrupt(PORT1_VECTOR)))
void PORT1_ISR(void)
{
	uint8_t detect = P1IFG & ALL_INPUTS;

    if (detect & SND_DETECT_FRONT) {
        port_last_event |= PORT_EVENT_IN0;
        _BIC_SR_IRQ(LPM3_bits);
    }

    if (detect & SND_DETECT_REAR) {
        port_last_event |= PORT_EVENT_IN1;
        _BIC_SR_IRQ(LPM3_bits);
    }

    // clear latest interrupt
	P1IV = 0x00;
}



