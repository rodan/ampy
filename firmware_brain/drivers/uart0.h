#ifndef __UART0_H__
#define __UART0_H__

#include "proj.h"

enum uart0_tevent {
    UART0_EV_RX = BIT0,
    UART0_EV_TX = BIT1
};

#define UART0_RXBUF_SZ     128

volatile char uart0_rx_buf[UART0_RXBUF_SZ];
volatile uint8_t uart0_p;
volatile uint8_t uart0_rx_buf_len;
volatile uint8_t uart0_rx_enable;
//uint8_t uart0_rx_err;

uint8_t xor_hash0;
uint8_t xor_hash0_active;

void uart0_init();
uint16_t uart0_tx_str(char *str, const uint16_t size);

void start_hash0(void);
void output_hash0(void);

volatile enum uart0_tevent uart0_last_event;

#endif
