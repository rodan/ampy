#ifndef __UART_FCTS_H__
#define __UART_FCTS_H__

#include "config.h"

#ifdef USE_UART

void save_presets(uint8_t location);

void load_presets(uint8_t location);

void get_mixer_status(void);

void mixer_send_funct(const uint8_t pga, const uint8_t function, const uint8_t r_diff, const uint8_t l_diff);

#endif
#endif
