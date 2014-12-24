#ifndef __I2C_FCTS_H__
#define __I2C_FCTS_H__

#include "config.h"

#ifdef USE_I2C

void save_presets(uint8_t location);

void load_presets(uint8_t location);

void get_mixer_status(void);

void mixer_send_funct(const uint8_t pga, const uint8_t function, const uint8_t r_diff, const uint8_t l_diff);

void i2c_tx_cmd(uint8_t cmd, uint8_t arg);
void i2c_tx_vol(uint8_t pga_id, uint8_t unmute, uint8_t vol_r, uint8_t vol_l);

#endif
#endif
