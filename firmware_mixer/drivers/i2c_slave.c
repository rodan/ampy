
#include <stdint.h>

#include "proj.h"
#include "i2c_slave.h"
#include "i2c_internal.h"

#ifdef I2C_SLAVE

__attribute__ ((interrupt(I2C_ISR_VECTOR)))
void USCI_BX_ISR(void)
{

    enum i2c_tevent ev = 0;

    switch (I2C_IV) {
    case 0:                     // Vector  0: No interrupts
        break;
    case 2:                     // Vector  2: ALIFG
        break;
    case 4:                     // Vector  4: NACKIFG
        break;
    case 6:                    // Vector  6: STTIFG
        I2C_IFG &= ~UCSTTIFG;   // Clear start condition int flag
        // feed data from this address onward
        i2c_slave_tx_data = i2c_slave_tx_data_start_addr;
        i2c_rx_ctr = 0;
        break;
    case 8:                    // Vector  8: STPIFG
        I2C_IFG &= ~UCSTPIFG;   // Clear stop condition int flag
        if (i2c_rx_ctr) {
            ev |= I2C_EV_RX;
        }
        __bic_SR_register_on_exit(LPM0_bits);   // Exit LPM0 if data was transmitted
        break;
    case 10:                   // Vector 10: RXIFG
        *i2c_slave_rx_data++ = I2C_RXBUF;
        i2c_rx_ctr++;
        break;
    case 12:                   // Vector 12: TXIFG
        I2C_TXBUF = *i2c_slave_tx_data++;  // Transmit data at address PTxData
        break;
    default:
        break;
    }
}

void i2c_slave_init(void)
{
    I2C_CTL0 = UCMODE_3 + UCSYNC;       // I2C Slave, synchronous mode
    I2C_CTL1 |= UCSWRST;        // Enable SW reset
    I2C_OA = I2C_SLAVE_ADDR;    // Own Address
    I2C_CTL1 &= ~UCSWRST;       // Clear SW reset, resume operation
    I2C_IE |= UCTXIE + UCRXIE + UCSTPIE + UCSTTIE;
}

void i2c_slave_uninit(void)
{
    I2C_CTL0 |= UCSWRST;
}

#endif
