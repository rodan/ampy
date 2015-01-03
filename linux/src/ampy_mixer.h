#ifndef __AMP_MIXER_H__
#define __AMP_MIXER_H__

#define VIEW_MIXER 0
#define VIEW_AMP   1

#include <sys/select.h>

extern const char view_mode_name[2][10];
extern const char card_name[2][22];
extern const char chip_name[2][26];

extern unsigned int tx_err;
extern unsigned int rx_err;
extern unsigned int tx_inval;

void main_loop(void);
int get_mixer_values(int *dev);
int set_mixer_volume(int *dev, const uint8_t pga_id, const uint8_t mute,
                     const uint8_t right, const uint8_t left);
int set_amp_registers(int *dev, const uint8_t ver, const uint8_t snd_detect, const uint8_t mute_flag);
int store_registers(int *dev, uint8_t type);

int fd_read_ready(int fd_dev, struct timeval* timeout);

int ampy_tx_cmd(int *fd_dev, char *tx_buff, uint8_t tx_buff_len, char *rx_buff, uint8_t *rx_buff_len, const uint8_t exp_rx_buff_len, const uint8_t retries);

uint8_t extract_hex(char *str, uint8_t * rv);

#endif
