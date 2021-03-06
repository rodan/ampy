
#include <string.h>
#include <stdio.h>

#include "uart0.h"

// you'll have to initialize/map uart ports in main()
void uart0_init(void)
{
    UCA0CTL1 |= UCSWRST;        // put state machine in reset
    UCA0CTL1 |= UCSSEL_1;       // use ACLK
    UCA0BR0 = 0x03;             // 32kHz/9600=3.41
    UCA0BR1 = 0x00;
    UCA0MCTL = UCBRS_3 + UCBRF_0;       // modulation UCBRSx=3, UCBRFx=0
    UCA0CTL1 &= ~UCSWRST;       // initialize USCI state machine
    UCA0IE |= UCRXIE;           // enable USCI_A0 RX interrupt
    uart0_p = 0;
    uart0_rx_enable = 1;
    xor_hash0_active = false;
}

void start_hash0(void)
{
    xor_hash0_active = true;
    xor_hash0 = 0;
}

void output_hash0(void)
{
    char hash_str[4];
    xor_hash0_active = false;
    snprintf(hash_str, 4, "*%02x", xor_hash0);
    uart0_tx_str(hash_str, strlen(hash_str));
}

uint16_t uart0_tx_str(char *str, const uint16_t size)
{
    uint16_t p = 0;
    while (p < size) {
        while (!(UCA0IFG & UCTXIFG)) ;  // USCI_A0 TX buffer ready?
        UCA0TXBUF = str[p];
        if (xor_hash0_active) {
            xor_hash0 ^= str[p];
        }
        p++;
    }
    return p;
}

__attribute__ ((interrupt(USCI_A0_VECTOR)))
void USCI_A0_ISR(void)
{
    uint16_t iv = UCA0IV;
    register char rx;
    enum uart0_tevent ev = 0;

    // iv is 2 for RXIFG, 4 for TXIFG
    switch (iv) {
    case 2:
        rx = UCA0RXBUF;
        if (uart0_rx_enable && (uart0_p < UART0_RXBUF_SZ-2)) {
            if (rx == 0x0a) {
                return;
            } else if (rx == 0x0d) {
                ev = UART0_EV_RX;
                uart0_rx_buf[uart0_p] = 0;
                uart0_rx_enable = false;
                _BIC_SR_IRQ(LPM3_bits);
            } else {
                uart0_rx_buf[uart0_p] = rx;
                uart0_p++;
            }
            uart0_rx_buf_len = uart0_p;
        } else {
            if ((rx == 0x0d) || (rx == 0x0a)) {
                uart0_p = 0;
            }
        }
        break;
    case 4:
        ev = UART0_EV_TX;
        break;
    default:
        break;
    }
    uart0_last_event |= ev;
}
