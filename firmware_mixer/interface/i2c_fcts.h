#ifndef __I2C_FCTS_H__
#define __I2C_FCTS_H__

#include "config.h"

#ifdef USE_I2C

void i2c_iface_init(void);

//uint8_t str_to_uint16(char *str, uint16_t * out, const uint8_t seek,
//    const uint8_t len, const uint16_t min, const uint16_t max);

#endif
#endif
