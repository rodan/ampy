
#include <stdio.h>
#include <string.h>

#include "drivers/uart0.h"
#include "drivers/timer_a0.h"
#include "drivers/pga2311_helper.h"
#include "interface/i2c_fcts.h"
#include "ui.h"
#ifdef HARDWARE_I2C
  #include "drivers/i2c.h"
#else
  #include "drivers/serial_bitbang.h"
#endif
void display_menu(void)
{
    //snprintf(str_temp, TEMP_LEN,
    //        "\r\n --- tracy build #%d\r\n  available commands:\r\n", BUILD);
    //uart0_tx_str(str_temp, strlen(str_temp));

    snprintf(str_temp, TEMP_LEN, " \e[33;1mstat\e[0m   - system status\r\n" );
    uart0_tx_str(str_temp, strlen(str_temp));

    snprintf(str_temp, TEMP_LEN, " \e[33;1mst\e[0m     - display &s\r\n" );
    uart0_tx_str(str_temp, strlen(str_temp));

    snprintf(str_temp, TEMP_LEN, " \e[33;1mrd\e[0m     - test read\r\n" );
    uart0_tx_str(str_temp, strlen(str_temp));

    snprintf(str_temp, TEMP_LEN, " \e[33;1mreset\e[0m  - uC reset\r\n" );
    uart0_tx_str(str_temp, strlen(str_temp));

}

void parse_user_input(void)
{
    char f = uart0_rx_buf[0];
    char *in = (char *) uart0_rx_buf;
    uint8_t i;

    if (f == '?') {
        display_menu();
    } else if (strstr(in, "stat")) {

        //get_mixer_status();
        for (i=0;i<6;i++) {
            snprintf(str_temp, TEMP_LEN, "pga%d %03d %03d %s\n", i+1, mixer_get_vol_struct(i+1, CH_RIGHT), mixer_get_vol_struct(i+1, CH_LEFT), mixer_get_mute_struct(i+1)?"1":"0");
            uart0_tx_str(str_temp, strlen(str_temp));
        }

        for (i=0;i<DETECT_CHANNELS;i++) {
            snprintf(str_temp, TEMP_LEN, "amp%d %s\n", i+1, stat.mute[i]?"0":"1");
            uart0_tx_str(str_temp, strlen(str_temp));
        }
    } else if (strstr(in, "reset")) {
        WDTCTL = WDTHOLD;
    } else if (strstr(in, "st")) {
        for (i=0;i<15;i++) {
            snprintf(str_temp, TEMP_LEN, "%d ", *((uint8_t *) &s+i));
            uart0_tx_str(str_temp, strlen(str_temp));
        }
        uart0_tx_str("\r\n", 2);
    } else if (strstr(in, "rd")) {
        memset(&s, 9, 15);
        get_mixer_status();
    }

            /*
            if (stat.mute[i]) {
                uart0_tx_str("\e[31;1moff\e[0m ", 15);
            } else {
                uart0_tx_str("\e[32;1mon\e[0m ", 14);
            }
        }
        uart0_tx_str("\r\n", 2);
        */
}

