
#include <stdint.h>
#include "port.h"
#include "proj.h"

//#define ALL_INPUTS  0x82

/*
void port_init(void)
{
	// IRQ triggers on a hi-low transition
	P1IES |= ALL_INPUTS;
	P1IFG &= ~ALL_INPUTS;
	P1IE |= ALL_INPUTS;
}
*/

__attribute__((interrupt(PORT1_VECTOR)))
void PORT1_ISR(void)
{
	input_ed = P1IFG & (detect_port[0] + detect_port[1]);

    if (input_ed) {
        port_last_event |= PORT_EVENT_TRIG;
        _BIC_SR_IRQ(LPM3_bits);
    }

    // clear latest interrupt
	P1IV = 0x00;
}

