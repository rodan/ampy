#ifndef __UART_H__
#define __UART_H__

#include "proj.h"

enum uart_tevent {
    UART_EV_RX = BIT0,
    UART_EV_TX = BIT1
};

#define UART_RXBUF_SZ     8

volatile char uart_rx_buf[UART_RXBUF_SZ];
volatile uint8_t uart_p;
uint8_t uart_rx_enable;
uint8_t uart_rx_err;

void uart_init();
uint16_t uart_tx_str(char *str, uint16_t size);

volatile enum uart_tevent uart_last_event;

#endif
