#ifndef __ADC_H__
#define __ADC_H__

#include "proj.h"

void adc12_read(const uint8_t port, uint16_t * rv, const uint8_t vref);
void adc12_halt(void);

int16_t calc_temp(const uint16_t qtemp);

#endif
