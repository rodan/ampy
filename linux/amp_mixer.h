#ifndef __AMP_MIXER_H__
#define __AMP_MIXER_H__

void main_loop(void);
int get_mixer_values(int dev);
int set_mixer_volume(int dev, const uint8_t pga_id, const uint8_t mute,
                     const uint8_t right, const uint8_t left);
uint8_t extract_hex(char *str, uint8_t * rv);

#endif
