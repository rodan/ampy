
#include <stdio.h>
#include <string.h>

#include "drivers/uart0.h"
#include "drivers/timer_a0.h"
#include "drivers/flash.h"
#include "drivers/pga2311_helper.h"
#include "drivers/lm4780_helper.h"
#include "drivers/adc.h"
#include "drivers/rtc.h"
#include "interface/i2c_fcts.h"
#include "string_helpers.h"
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

    snprintf(str_temp, TEMP_LEN, " \e[33;1msensors\e[0m   - display internal sensors\r\n" );
    uart0_tx_str(str_temp, strlen(str_temp));

    snprintf(str_temp, TEMP_LEN, "uptime %lds\r\n", rtca_time.sys );
    uart0_tx_str(str_temp, strlen(str_temp));

}

int parse_user_input(void)
{
    char f = uart0_rx_buf[0];
    char *in = (char *) uart0_rx_buf;
    uint8_t i;
    uint8_t t_int[4];
    uint8_t *flash_addr = FLASH_ADDR;
    uint16_t q_t_brain = 0; //, t_mixer;

    if (f == '?') {
        display_menu();
    } else if (strstr(in, "reset")) {
        WDTCTL = WDTHOLD;
    } else if (strstr(in, "stat")) {
        get_mixer_status();
        for (i=0;i<6;i++) {
            snprintf(str_temp, TEMP_LEN, "pga%d %03d %03d %s\n", i+1, mixer_get_vol_struct(i+1, CH_RIGHT), mixer_get_vol_struct(i+1, CH_LEFT), mixer_get_mute_struct(i+1)?"A":"M");
            uart0_tx_str(str_temp, strlen(str_temp));
        }
        for (i=0;i<DETECT_CHANNELS;i++) {
            snprintf(str_temp, TEMP_LEN, "amp%d detect %s\n", i+1, ampy_get_detect(i+1)?"A":"M");
            uart0_tx_str(str_temp, strlen(str_temp));
        }
        for (i=0;i<DETECT_CHANNELS;i++) {
            snprintf(str_temp, TEMP_LEN, "amp%d default %s\n", i+1, ampy_get_mute(i+1)?"A":"M");
            uart0_tx_str(str_temp, strlen(str_temp));
        }
        for (i=0;i<DETECT_CHANNELS;i++) {
            snprintf(str_temp, TEMP_LEN, "amp%d current %s\n", i+1, ampy_get_status(i+1)?"A":"M");
            uart0_tx_str(str_temp, strlen(str_temp));
        }
    } else if (strstr(in, "showreg")) {
        // mixer related
        get_mixer_status(); // XXX
        start_hash0();
        for (i=0;i<14;i++) {
            snprintf(str_temp, TEMP_LEN, "%02x", *((uint8_t *) &s+i));
            uart0_tx_str(str_temp, strlen(str_temp));
        }
        // power amp related
        for (i=0;i<4;i++) {
            snprintf(str_temp, TEMP_LEN, "%02x", *((uint8_t *) &a+i));
            uart0_tx_str(str_temp, strlen(str_temp));
        }
        output_hash0();
        uart0_tx_str(" ok\r\n", 5);
    } else if (strstr(in, "storeamp")) {
        flash_save(flash_addr, (void *)&a, 3);
        settings_apply();
        uart0_tx_str("ok\r\n", 4);
    } else if (strstr(in, "storemix")) {
        i2c_tx_cmd(M_CMD_WRITE, 1);
        uart0_tx_str("ok\r\n", 4);
    } else if (strstr(in, "sensor")) {
        adc12_read(10, &q_t_brain, REFVSEL_0);
        adc12_halt();
        snprintf(str_temp, TEMP_LEN, "T_BRAIN %d T_MIXER %d ok\r\n", calc_temp(q_t_brain), (int8_t) s.int_temp);
        uart0_tx_str(str_temp, strlen(str_temp));
    } else if (f == 'v') {
        // receive volume levels - one line per pga
        if (check_xor_hash(in, uart0_rx_buf_len) == EXIT_SUCCESS) {
            for (i=0;i<4;i++) {
                if (extract_hex((char *)uart0_rx_buf+i*2+1, &t_int[i]) != 2) {
                    uart0_tx_str("extract_hex fail\r\n", 10);
                    return EXIT_FAILURE;
                }
            }
            i2c_tx_vol(t_int[0], t_int[1], t_int[2], t_int[3]);
            uart0_tx_str(in, uart0_rx_buf_len);
            uart0_tx_str(" ok\r\n", 5);
        } else {
            uart0_tx_str("hash fail\r\n", 11);
        }
    } else if (f == 'a') {
        // receive amp settings
        if (check_xor_hash(in, uart0_rx_buf_len) == EXIT_SUCCESS) {
            for (i=0;i<3;i++) {
                if (extract_hex((char *)uart0_rx_buf+i*2+1, (uint8_t *) &a+i) != 2) {
                    uart0_tx_str("extract_hex fail\r\n", 10);
                    return EXIT_FAILURE;
                }
            }
            uart0_tx_str(in, uart0_rx_buf_len);
            settings_apply();
            uart0_tx_str(" ok\r\n", 5);
        } else {
            uart0_tx_str("hash fail\r\n", 11);
        }
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
    return EXIT_SUCCESS;
}

