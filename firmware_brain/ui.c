
#include <stdio.h>
#include <string.h>

#include "drivers/uart0.h"
#include "drivers/timer_a0.h"
#include "drivers/flash.h"
#include "drivers/pga2311_helper.h"
#include "drivers/lm4780_helper.h"
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

    snprintf(str_temp, TEMP_LEN, " \e[33;1mstat\e[0m      - system status\r\n" );
    uart0_tx_str(str_temp, strlen(str_temp));

    snprintf(str_temp, TEMP_LEN, " \e[33;1mshowreg\e[0m   - display &s and &a\r\n" );
    uart0_tx_str(str_temp, strlen(str_temp));

    snprintf(str_temp, TEMP_LEN, " \e[33;1maXXXXXX\e[0m   - set amp ver, snd_det, mute_flag\r\n" );
    uart0_tx_str(str_temp, strlen(str_temp));

    snprintf(str_temp, TEMP_LEN, " \e[33;1mvXXXXXXXX\e[0m - set vol pga_id, mute, volr, voll\r\n" );
    uart0_tx_str(str_temp, strlen(str_temp));

    snprintf(str_temp, TEMP_LEN, " \e[33;1mstoreamp\e[0m  - save amp regs to flash\r\n" );
    uart0_tx_str(str_temp, strlen(str_temp));

    snprintf(str_temp, TEMP_LEN, " \e[33;1mstoremix\e[0m  - save mixer regs to flash\r\n" );
    uart0_tx_str(str_temp, strlen(str_temp));

    snprintf(str_temp, TEMP_LEN, " \e[33;1mreset\e[0m     - uC reset\r\n" );
    uart0_tx_str(str_temp, strlen(str_temp));

}

uint8_t extract_hex(char *str, uint8_t *rv)
{
    uint8_t i=0;
    char *p = str;
    char c = *p;
    
    *rv = 0;

    while ((i<2) && (((c > 47) && (c < 58)) || ((c > 96) && (c < 103)) || ((c > 64) && (c < 71)))) {

        // go lowercase (A-F -> a-f)
        if ((c > 64) && (c < 71)) {
            c += 32;
        }

        *rv = *rv << 4;
        if ((c > 47) && (c < 58)) {
            *rv += c - 48;
        } else if ((c > 96) && (c < 103)) {
            *rv += c - 87;
        }
        i++;
        //p++;
        c = *++p;
    }

    return i;
}

void parse_user_input(void)
{
    char f = uart0_rx_buf[0];
    char *in = (char *) uart0_rx_buf;
    uint8_t i;
    uint8_t t_int[4];
    uint8_t *flash_addr = FLASH_ADDR;


    if (f == '?') {
        display_menu();
    } else if (strstr(in, "reset")) {
        WDTCTL = WDTHOLD;
    } else if (strstr(in, "stat")) {
        get_mixer_status();
        for (i=0;i<6;i++) {
            snprintf(str_temp, TEMP_LEN, "pga%d %03d %03d %s\n", i+1, mixer_get_vol_struct(i+1, CH_RIGHT), mixer_get_vol_struct(i+1, CH_LEFT), mixer_get_mute_struct(i+1)?"1":"0");
            uart0_tx_str(str_temp, strlen(str_temp));
        }
        for (i=0;i<DETECT_CHANNELS;i++) {
            snprintf(str_temp, TEMP_LEN, "amp%d detect %s\n", i+1, ampy_get_detect(i+1)?"0":"1");
            uart0_tx_str(str_temp, strlen(str_temp));
        }
        for (i=0;i<DETECT_CHANNELS;i++) {
            snprintf(str_temp, TEMP_LEN, "amp%d default %s\n", i+1, ampy_get_mute(i+1)?"0":"1");
            uart0_tx_str(str_temp, strlen(str_temp));
        }
        for (i=0;i<DETECT_CHANNELS;i++) {
            snprintf(str_temp, TEMP_LEN, "amp%d current %s\n", i+1, ampy_get_status(i+1)?"0":"1");
            uart0_tx_str(str_temp, strlen(str_temp));
        }
    } else if (strstr(in, "showreg")) {
        // mixer related
        get_mixer_status();
        for (i=0;i<14;i++) {
            snprintf(str_temp, TEMP_LEN, "%02x", *((uint8_t *) &s+i));
            uart0_tx_str(str_temp, strlen(str_temp));
        }
        // power amp related
        for (i=0;i<4;i++) {
            snprintf(str_temp, TEMP_LEN, "%02x", *((uint8_t *) &a+i));
            uart0_tx_str(str_temp, strlen(str_temp));
        }
        uart0_tx_str("\r\n", 2);
    } else if (strstr(in, "storeamp")) {
        flash_save(flash_addr, (void *)&a, 3);
        settings_apply();
    } else if (strstr(in, "storemix")) {
        i2c_tx_cmd(M_CMD_WRITE, 1);
    } else if (f == 'v') {
        // receive volume levels - one line per pga
        for (i=0;i<4;i++) {
            extract_hex((char *)uart0_rx_buf+i*2+1, &t_int[i]);
        }
        i2c_tx_vol(t_int[0], t_int[1], t_int[2], t_int[3]);
    } else if (f == 'a') {
        // receive amp settings
        for (i=0;i<3;i++) {
            extract_hex((char *)uart0_rx_buf+i*2+1, (uint8_t *) &a+i);
        }
        settings_apply();
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

