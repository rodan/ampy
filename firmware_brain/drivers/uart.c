
// hardware UART implementation that uses P1.5 as RXI and P1.6 for TXO

#include "uart.h"

void uart_init(void)
{
    // hardware UART
    PMAPPWD = 0x02D52;                        // Get write-access to port mapping regs
    P1MAP5 = PM_UCA0RXD;                      // Map UCA0RXD output to P1.5
    P1MAP6 = PM_UCA0TXD;                      // Map UCA0TXD output to P1.6
    PMAPPWD = 0;                              // Lock port mapping registers
 
    P1DIR |= BIT6;                            // Set P1.6 as TX output
    P1SEL |= BIT5 + BIT6;                     // Select P1.5 & P1.6 to UART function
 
    UCA0CTL1 |= UCSWRST;                      // **Put state machine in reset**
    UCA0CTL1 |= UCSSEL_1;                     // CLK = ACLK
    UCA0BR0 = 0x03;                           // 32kHz/9600=3.41 (see User's Guide)
    UCA0BR1 = 0x00;                           //
    UCA0MCTL = UCBRS_3+UCBRF_0;               // Modulation UCBRSx=3, UCBRFx=0
    UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
    UCA0IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt
    uart_p = 0;
    uart_rx_enable = 1;
    uart_rx_err = 0;
}

uint16_t uart_tx_str(char *str, uint16_t size)
{
    uint16_t p = 0;
    while (p < size) {
        while (!(UCA0IFG & UCTXIFG)) ;  // USCI_A0 TX buffer ready?
        UCA0TXBUF = str[p];
        p++;
    }
    return p;
}

__attribute__ ((interrupt(USCI_A0_VECTOR)))
void USCI_A0_ISR(void)
{
    uint16_t iv = UCA0IV;
    enum uart_tevent ev = 0;
    register char rx;

    // iv is 2 for RXIFG, 4 for TXIFG
    switch (iv) {
    case 2:
        rx = UCA0RXBUF;
        if (uart_rx_enable && !uart_rx_err && (uart_p < UART_RXBUF_SZ-2)) {
            if (rx == 0x0d) {
                return;
            } else if (rx == 0x0a) {
                ev = UART_EV_RX;
                uart_rx_buf[uart_p] = 0;
                uart_rx_enable = 0;
                uart_rx_err = 0;
                _BIC_SR_IRQ(LPM3_bits);
            } else {
                uart_rx_buf[uart_p] = rx;
                uart_p++;
            }
        } else {
            uart_rx_err++;
            if (rx == 0x0a) {
                uart_rx_err = 0;
                uart_p = 0;
            }
        }
        break;
    case 4:
        ev = UART_EV_TX;
        break;
    default:
        break;
    }
    uart_last_event |= ev;
}

