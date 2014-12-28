#ifndef __AMP_MIXER_H__
#define __AMP_MIXER_H__

void main_loop(void);
int get_mixer_values(int *dev);
int set_mixer_volume(int *dev, const uint8_t pga_id, const uint8_t mute,
                     const uint8_t right, const uint8_t left);
uint8_t extract_hex(char *str, uint8_t * rv);

void initialize_curses(uint8_t use_color);

//extern const char ch_name[6][8];
//extern const char ch_name_long[6][20];

extern int view_mode;
#define VIEW_MIXER 0
#define VIEW_AMP   1

extern const char view_mode_name[2][10];
extern const char card_name[2][22];
extern const char chip_name[2][26];

#endif
