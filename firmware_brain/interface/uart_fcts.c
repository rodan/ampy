
//  audio mixer based on an MSP430F5510
//  deprecated functions that used UART to talk to the mixer board.
//  i2c_fcts should be used instead
//
//  author:          Petre Rodan <2b4eda@subdimension.ro>
//  available from:  https://github.com/rodan/ampy
//  license:         GNU GPLv3

#include "config.h"

#ifdef USE_UART

#include <stdio.h>
#include <string.h>
#include "proj.h"
#include "drivers/uart.h"
#include "drivers/pga2311_helper.h"

void save_presets(uint8_t location)
{
    snprintf(str_temp, TEMP_LEN, "w%d\n", location);
    uart_tx_str(str_temp, strlen(str_temp));
}

void load_presets(uint8_t location)
{
    snprintf(str_temp, TEMP_LEN, "r%d\n", location);
    uart_tx_str(str_temp, strlen(str_temp));
}

void get_mixer_status(void)
{
    uart_tx_str("s\n", 2);
}

void mixer_send_funct(const uint8_t pga, const uint8_t function, const uint8_t r_diff, const uint8_t l_diff) 
{
    uint8_t vol_r, vol_l;

    switch (function) {

    case FCT_T_MUTE:
        if (!mixer_get_mute_struct(pga)) {
            mixer_set_mute_struct(pga, UNMUTE);
            snprintf(str_temp, TEMP_LEN, "u%d\n", pga);
        } else {
            mixer_set_mute_struct(pga, MUTE);
            snprintf(str_temp, TEMP_LEN, "m%d\n", pga);
        }
        uart_tx_str(str_temp, strlen(str_temp));
    break;

    case FCT_V_INC:
        vol_r = mixer_get_vol_struct(pga, CH_RIGHT);
        if (vol_r < 255 - r_diff) {
            vol_r+=r_diff;
            mixer_set_vol_struct(pga, CH_RIGHT, vol_r);
        }
        vol_l = mixer_get_vol_struct(pga, CH_LEFT);
        if (vol_l < 255 - l_diff) {
            vol_l+=l_diff;
            mixer_set_vol_struct(pga, CH_LEFT, vol_l);
        }
        if (vol_r == vol_l) {
            snprintf(str_temp, TEMP_LEN, "v%db%d\n", pga, vol_r);
            uart_tx_str(str_temp, strlen(str_temp));
        } else {
            snprintf(str_temp, TEMP_LEN, "v%dr%d\n", pga, vol_r);
            uart_tx_str(str_temp, strlen(str_temp));

            snprintf(str_temp, TEMP_LEN, "v%dl%d\n", pga, vol_l);
            uart_tx_str(str_temp, strlen(str_temp));
        }
    break;

    case FCT_V_DEC:
        vol_r = mixer_get_vol_struct(pga, CH_RIGHT);
        if (vol_r > r_diff) {
            vol_r-=r_diff;
            mixer_set_vol_struct(pga, CH_RIGHT, vol_r);
        }
        vol_l = mixer_get_vol_struct(pga, CH_LEFT);
        if (vol_l > l_diff) {
            vol_l-=l_diff;
            mixer_set_vol_struct(pga, CH_LEFT, vol_l);
        }
        if (vol_r == vol_l) {
            snprintf(str_temp, TEMP_LEN, "v%db%d\n", pga, vol_r);
            uart_tx_str(str_temp, strlen(str_temp));
        } else {
            snprintf(str_temp, TEMP_LEN, "v%dr%d\n", pga, vol_r);
            uart_tx_str(str_temp, strlen(str_temp));

            snprintf(str_temp, TEMP_LEN, "v%dl%d\n", pga, vol_l);
            uart_tx_str(str_temp, strlen(str_temp));
        }
    break;
    }
}

#endif
