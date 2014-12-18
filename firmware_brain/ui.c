
#include <stdio.h>
#include <string.h>

#include "drivers/uart0.h"
#include "drivers/timer_a0.h"
#include "ui.h"

void display_menu(void)
{
    //snprintf(str_temp, TEMP_LEN,
    //        "\r\n --- tracy build #%d\r\n  available commands:\r\n", BUILD);
    //uart0_tx_str(str_temp, strlen(str_temp));

    snprintf(str_temp, TEMP_LEN, " \e[33;1mstat\e[0m          - system status\r\n" );
    uart0_tx_str(str_temp, strlen(str_temp));

}

void parse_user_input(void)
{
    char f = uart0_rx_buf[0];
    char *in = (char *) uart0_rx_buf;
    //uint8_t *src_p;
    uint8_t i;
    //uint8_t j;
    //uint8_t row[8];
    //uint8_t zeroes[128];

    if (f == '?') {
        display_menu();
    } else if (strstr(in, "stat")) {
        for (i=0;i<DETECT_CHANNELS;i++) {
            snprintf(str_temp, TEMP_LEN, " ch%d: ", i);
            uart0_tx_str(str_temp, strlen(str_temp));
            if (stat.mute[i]) {
                uart0_tx_str("\e[31;1moff\e[0m ", 15);
            } else {
                uart0_tx_str("\e[32;1mon\e[0m ", 14);
            }
        }
        uart0_tx_str("\r\n", 2);
    }
}

