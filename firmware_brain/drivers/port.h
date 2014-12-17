
#ifndef __DETECTION_H__
#define __DETECTION_H__

#include "proj.h"

#define SND_DETECT_FRONT    (BIT7)
#define SND_DETECT_REAR     (BIT1)

volatile uint8_t detection_state;

void port_init(void);

enum port_event {
    PORT_EVENT_IN0 = BIT0,
    PORT_EVENT_IN1 = BIT1
};

volatile enum port_event port_last_event;

#endif
