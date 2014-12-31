#ifndef MIXER_CONTROLS_H_INCLUDED
#define MIXER_CONTROLS_H_INCLUDED

#include <inttypes.h>

#define CONTROLS_MAX_COUNT 7

struct control {
	char *name;
    char *full_name;
	unsigned int flags;
#define TYPE_PVOLUME	(1u << 4)
#define TYPE_PSWITCH	(1u << 6)
#define TYPE_ENUM	    (1u << 8)
#define IS_ACTIVE	    (1u << 16)
	uint8_t *volume_left;
	uint8_t *volume_right;
	uint8_t *pswitch;
};

uint8_t mixer_mute[6];

uint8_t amp[6];

extern struct control controls[2][CONTROLS_MAX_COUNT];
int controls_count;

void create_controls(void);

#endif

