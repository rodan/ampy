
//  audio mixer based on an MSP430F5510
//
//  author:          Petre Rodan <petre.rodan@simplex.ro>
//  available from:  https://github.com/rodan/ampy
//  license:         GNU GPLv3

#include "config.h"

#ifdef USE_I2C

#include <stdio.h>
#include <string.h>
#include "proj.h"
#include "drivers/timer_a0.h"
#include "drivers/flash.h"
#include "drivers/sys_messagebus.h"
#include "drivers/pga2311.h"
#include "i2c_slave.h"
#include "i2c_fcts.h"

static void i2c_rx_irq(enum sys_message msg)
{
    uint8_t *flash_addr = FLASH_ADDR;

    uint8_t cmd = i2c_rx_buff[0];
    uint8_t arg = i2c_rx_buff[1];
    uint8_t mute_st = i2c_rx_buff[2];
    uint8_t vol_r = i2c_rx_buff[3];
    uint8_t vol_l = i2c_rx_buff[4];

    switch (cmd) {

        case M_CMD_WRITE:
            if (arg == 2) {
                flash_addr = SEGMENT_C;
            } else if (arg == 3) {
                flash_addr = SEGMENT_D;
            } else {
                flash_addr = SEGMENT_B;
            }
            flash_save(flash_addr, (void *)&s, sizeof(s));
            break;

        case M_CMD_READ:
            if (arg == 2) {
                flash_addr = SEGMENT_C;
            } else if (arg == 3) {
                flash_addr = SEGMENT_D;
            } else {
                flash_addr = SEGMENT_B;
            }
            settings_init(flash_addr);
            break;

        case M_CMD_VOL:
            if ((0 < arg) && (arg < 7) && ((mute_st == MUTE) || (mute_st == UNMUTE))) {
                pga_set_volume(arg, vol_r, vol_l, 1, 1);
                if (mute_st != mixer_get_mute_struct(arg)) {
                    if (mute_st == MUTE) {
                        pga_set_mute_st(arg, MUTE, 1);
                    } else { 
                        pga_set_mute_st(arg, UNMUTE, 1);
                    }
                }
            }
            break;

        case M_CMD_MUTE:
            if ((0 < arg) && (arg < 7)) {
                pga_set_mute_st(arg, MUTE, 1);
            }
            break;

        case M_CMD_UNMUTE:
            if ((0 < arg) && (arg < 7)) {
                pga_set_mute_st(arg, UNMUTE, 1);
            }
            break;
    }

    i2c_rx_rdy = 1;
}

void i2c_iface_init(void)
{
    sys_messagebus_register(&i2c_rx_irq, SYS_MSG_I2C_RX);

    // when the i2c data request comes in this is the address 
    // from which to start sending
    i2c_slave_tx_data_start_addr = (uint8_t *) &s;
    i2c_slave_rx_data_start_addr = i2c_rx_buff;
    i2c_rx_rdy = 1;
}


#endif

