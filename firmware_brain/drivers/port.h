
#ifndef __DETECTION_H__
#define __DETECTION_H__

#include "proj.h"

//#define SND_DETECT_FRONT    (BIT7)
//#define SND_DETECT_REAR     (BIT1)

volatile uint8_t input_ed;  // edge detected input

void port_init(void);

enum port_event {
    PORT_EVENT_TRIG = BIT0
};

volatile enum port_event port_last_event;

#endif
