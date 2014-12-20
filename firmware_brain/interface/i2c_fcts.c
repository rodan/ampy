
//  audio mixer based on an MSP430F5510
//
//  author:          Petre Rodan <petre.rodan@simplex.ro>
//  available from:  https://github.com/rodan/ampy
//  license:         GNU GPLv3

#include "config.h"

#ifdef USE_I2C

#include "proj.h"
#include "drivers/pga2311_helper.h"

    #include "serial_bitbang.h"
#ifdef HARDWARE_I2C
    #include "drivers/i2c.h"
#endif


// XXX
//#include <stdio.h>
//#include <string.h>
//#include "drivers/uart.h"

void i2c_tx_cmd(uint8_t cmd, uint8_t arg)
{
    i2c_tx_buff[0] = cmd;
    i2c_tx_buff[1] = arg;

#ifdef HARDWARE_I2C
    pkg.slave_addr = I2C_MIXER_SLAVE_ADDR;
    pkg.addr[0] = 0;
    pkg.addr_len = 0;

    pkg.data = i2c_tx_buff;
    pkg.data_len = 2;
    pkg.read = 0;

    i2c_transfer_start(&pkg, NULL);
#else

    i2cm_tx_buff(I2C_MIXER_SLAVE_ADDR, (uint8_t *) &i2c_tx_buff, 2);

#endif
}

void i2c_tx_vol(uint8_t pga_id, uint8_t unmute, uint8_t vol_r, uint8_t vol_l)
{

    i2c_tx_buff[0] = M_CMD_VOL;
    i2c_tx_buff[1] = pga_id;
    i2c_tx_buff[2] = unmute;
    i2c_tx_buff[3] = vol_r;
    i2c_tx_buff[4] = vol_l;

#ifdef HARDWARE_I2C
    pkg.slave_addr = I2C_MIXER_SLAVE_ADDR;
    pkg.addr[0] = 0;
    pkg.addr_len = 0;

    i2c_tx_buff[0] = M_CMD_VOL;
    i2c_tx_buff[1] = pga_id;
    i2c_tx_buff[2] = unmute;
    i2c_tx_buff[3] = vol_r;
    i2c_tx_buff[4] = vol_l;

    pkg.data = i2c_tx_buff;
    pkg.data_len = 5;
    pkg.read = 0;

    i2c_transfer_start(&pkg, NULL);
#else

    i2cm_tx_buff(I2C_MIXER_SLAVE_ADDR, (uint8_t *) &i2c_tx_buff, 5);

#endif
}


void save_presets(uint8_t location)
{
    i2c_tx_cmd(M_CMD_WRITE, location);
}

void load_presets(uint8_t location)
{
    i2c_tx_cmd(M_CMD_READ, location);
}

void get_mixer_status(void)
{
    uint8_t buff[14];
    uint8_t rv;
    uint8_t i;
    uint8_t *src_p, *dst_p;

#ifdef HARDWARE_I2C
    pkg.slave_addr = I2C_MIXER_SLAVE_ADDR;
    pkg.addr[0] = 0;
    pkg.addr_len = 0;

    pkg.data = (uint8_t *) &s;
    pkg.data_len = 14;
    pkg.read = 1;

    i2c_transfer_start(&pkg, NULL);
    rv = I2C_ACK;
#else

    rv = i2cm_rx_buff(I2C_MIXER_SLAVE_ADDR, (uint8_t *) &buff, 14);

#endif

    if (rv == I2C_ACK) {
        src_p = buff;
        dst_p = (uint8_t *) & s;
        for (i=0;i<14;i++) {
            *dst_p++ = *src_p++;
        }
    }
}

void mixer_send_funct(const uint8_t pga, const uint8_t function, const uint8_t r_diff, const uint8_t l_diff) 
{
    uint8_t vol_r, vol_l;
    uint8_t vol_new_r, vol_new_l;
    uint8_t unmuted;
    uint8_t change = 0;

    if (pga < 1 || pga > 6) {
        return;
    }

    unmuted = mixer_get_mute_struct(pga);
    vol_r = mixer_get_vol_struct(pga, CH_RIGHT);
    vol_l = mixer_get_vol_struct(pga, CH_LEFT);
    //sprintf(str_temp, "D1 %d %d %d %d\n", pga, function, vol_r, vol_l);
    //uart_tx_str(str_temp, strlen(str_temp));

    switch (function) {

    case FCT_T_MUTE:
        if (!unmuted) {
            mixer_set_mute_struct(pga, UNMUTE);
            i2c_tx_vol(pga, UNMUTE, vol_r, vol_l);
        } else {
            mixer_set_mute_struct(pga, MUTE);
            i2c_tx_vol(pga, MUTE, vol_r, vol_l);
        }
    break;

    case FCT_V_INC:
        if (vol_r < 255 - r_diff) {
            vol_new_r = vol_r + r_diff;
        } else {
            vol_new_r = 255;
        }
        if (vol_r != vol_new_r) {
            mixer_set_vol_struct(pga, CH_RIGHT, vol_new_r);
            change = 1;
        }

        if (vol_l < 255 - l_diff) {
            vol_new_l = vol_l + l_diff;
        } else {
            vol_new_l = 255;
        }
        if (vol_l != vol_new_l) {
            mixer_set_vol_struct(pga, CH_LEFT, vol_new_l);
            change = 1;
        }
        if (change) {
            //sprintf(str_temp, "D2 %d %d %d %d\n", pga, unmuted, vol_new_r, vol_new_l);
            //uart_tx_str(str_temp, strlen(str_temp));
            i2c_tx_vol(pga, unmuted, vol_new_r, vol_new_l);
        }
    break;

    case FCT_V_DEC:
        if (vol_r > r_diff) {
            vol_new_r = vol_r - r_diff;
        } else {
            vol_new_r = 0;
        }
        if (vol_r != vol_new_r) {
            mixer_set_vol_struct(pga, CH_RIGHT, vol_new_r);
            change = 1;
        }
        if (vol_l > l_diff) {
            vol_new_l = vol_l - l_diff;
        } else {
            vol_new_l = 0;
        }
        if (vol_l != vol_new_l) {
            mixer_set_vol_struct(pga, CH_LEFT, vol_new_l);
            change = 1;
        }
        if (change) {
            //sprintf(str_temp, "D3 %d %d %d %d\n", pga, unmuted, vol_new_r, vol_new_l);
            //uart_tx_str(str_temp, strlen(str_temp));
            i2c_tx_vol(pga, unmuted, vol_new_r, vol_new_l);
        }
    break;
    }
}

#endif
