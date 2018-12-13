
//  read adc conversions from any of the P6 ports
//
//  at least a 1ms delay should be inserted between two adc10_read()s or
//  between an adc10_read(port, &rv) and the use of rv.
//
//  author:          Petre Rodan <2b4eda@subdimension.ro>
//  available from:  https://github.com/rodan/
//  license:         GNU GPLv3

#include "adc.h"

volatile uint16_t *adc12_rv;
volatile uint8_t adcready;

// port: 0 = P6.A0, 1 = P6.A1, .., 0xa = P6.A10 = internal temp sensor
// vref is one of:  REFVSEL_0  - 1.5v vref
//                  REFVSEL_1  - 2.0v vref
//                  REFVSEL_2  - 2.5v vref
void adc12_read(const uint8_t port, uint16_t * rv, const uint8_t vref)
{
    //*((uint16_t *)portreg) |= 1 << port;
    // if ref or adc12 are busy then wait
    //while (REFCTL0 & REFGENBUSY) ;
    while (ADC12CTL1 & ADC12BUSY) ;
    // enable reference
    if ((REFCTL0 & 0x30) != vref) {
        // need to change vref
        REFCTL0 &= ~(0x30 + REFON);
        REFCTL0 |= REFMSTR + vref + REFON;
    } else {
        REFCTL0 |= REFMSTR + REFON;
    }
    ADC12CTL0 &= ~ADC12ENC;

    ADC12CTL0 = ADC12SHT0_8 + ADC12ON; // set sample time
    ADC12CTL1 = ADC12SHP; // + ADC12DIV1 + ADC12DIV0; // enable sample timer
    ADC12MCTL0 = ADC12SREF_1 + port; // select ADC input
    //ADC12CTL2 |= ADC12PDIV_2 + ADC12SR;
    adcready = 0;
    adc12_rv = rv;
    // trigger conversion
    ADC12IE = ADC12IE0;
    ADC12CTL0 |= ADC12ENC + ADC12SC; // sampling and conversion start
    while (!adcready) ;
}

// calculate internal temperature based on the linear regression 
// established by the two calibration registers flashed into the chip
// qtemp the adc value on channel 10 with a 1.5V reference
// function returns the temperature in degrees C
int16_t calc_temp(const uint16_t qtemp)
{
    uint16_t x1 = *(uint16_t *)0x1a1a; // value at 30dC
    uint16_t x2 = *(uint16_t *)0x1a1c; // value at 85dC, see datasheet
    uint16_t y1 = 30;
    uint16_t y2 = 85;
    int32_t sumxsq;
    int32_t sumx, sumy, sumxy;
    int32_t coef1, coef2, t10;
    int32_t rv = 0;

    sumx = x1 + x2;
    sumy = y1 + y2;
    sumxsq = (int32_t)x1 * (int32_t)x1 + (int32_t)x2 * (int32_t)x2;
    sumxy = (int32_t)x1 * (int32_t)y1 + (int32_t)x2 * (int32_t)y2;

    coef1 = ((sumy*sumxsq)-(sumx*sumxy))/((2*sumxsq)-(sumx*sumx))*100;
    coef2 = 100*((2*sumxy)-(sumx*sumy))/((2*sumxsq)-(sumx*sumx));

    t10 = (qtemp * coef2 + coef1)/10;
    rv = t10/10;

    // add 1 if first digit after decimal is > 4
    if ( (t10 % 10) > 4 ) {
        if (t10 > 0) {
            rv += 1;
        } else {
            rv -= 1;
        }
    }
    return rv;
}

void adc12_halt(void)
{
    ADC12CTL0 &= ~ADC12ON;
    REFCTL0 &= ~REFON;
}

__attribute__ ((interrupt(ADC12_VECTOR)))
void adc12_ISR(void)
{
    uint16_t iv = ADC12IV;
    if (iv == ADC12IV_ADC12IFG0) {
        *adc12_rv = ADC12MEM0;
        adcready = 1;
    }
}
