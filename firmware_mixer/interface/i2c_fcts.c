
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

void i2c_iface_init(void)
{
    //sys_messagebus_register(&i2c_rx_irq, SYS_MSG_I2C_RX);

    // when the i2c data request comes in this is the address 
    // from which to start sending
    i2c_slave_tx_data_start_addr = (uint8_t *) &s;
}


#endif

