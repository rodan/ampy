
#include <stdio.h>
#include "mixer_controls.h"
#include "mixer_display.h"
#include "amp_mixer.h"
#include "pga2311_helper.h"

//int controls_count = 6;

struct control controls[2][CONTROLS_MAX_COUNT] = {
    {
        {"front", "front", TYPE_PVOLUME | TYPE_PSWITCH | IS_ACTIVE, &s.v[1], &s.v[0], &mixer_mute[0]},
        {"rear", "rear", TYPE_PVOLUME | TYPE_PSWITCH | IS_ACTIVE, &s.v[3], &s.v[2], &mixer_mute[1]},
        {"line", "line", TYPE_PVOLUME | TYPE_PSWITCH | IS_ACTIVE, &s.v[5], &s.v[4], &mixer_mute[2]},
        {"spdif", "spdif", TYPE_PVOLUME | TYPE_PSWITCH | IS_ACTIVE, &s.v[7], &s.v[6], &mixer_mute[3]},
        {"pan", "front to rear pan", TYPE_PVOLUME | TYPE_PSWITCH | IS_ACTIVE, &s.v[9], &s.v[8], &mixer_mute[4]},
        {"c/subw", "center/subwoofer", TYPE_PVOLUME | TYPE_PSWITCH | IS_ACTIVE, &s.v[11], &s.v[10], &mixer_mute[5]},
        {NULL, NULL, 0, NULL, NULL} // keep as last control
    },{
        {"snd det", "sound detection", TYPE_PSWITCH | IS_ACTIVE, NULL, NULL, &amp_detect_en},
        {"front", "front", TYPE_PSWITCH, NULL, NULL, &amp_mute[0]},
        {"rear", "rear", TYPE_PSWITCH, NULL, NULL, &amp_mute[1]},
        {NULL, NULL, 0, NULL, NULL} // keep as last control
    }
};

void create_controls(void) 
{
    uint8_t i;

    if (view_mode == VIEW_MIXER) {
        for (i=0;i<6;i++) {
            mixer_mute[i] = mixer_get_mute_struct(i + 1);
        }
    }

    controls_count = 0;
    while (controls[view_mode][controls_count].name) {
        controls_count++;
        if (controls_count > CONTROLS_MAX_COUNT) {
            controls_count = 0;
            break;
        }
    }

    compute_controls_layout();
    display_view_mode();

    //search_for_focus_control();
	//refocus_control();
}

