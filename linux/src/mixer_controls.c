/*
 * mixer_controls.c - handles mixer controls and mapping from selems
 * Copyright (c) 1998,1999 Tim Janik
 *                         Jaroslav Kysela <perex@perex.cz>
 * Copyright (c) 2009      Clemens Ladisch <clemens@ladisch.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include "mixer_controls.h"
#include "mixer_display.h"
#include "mixer_widget.h"
#include "ampy_mixer.h"
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
        {"f detect", "front sound detection", TYPE_PSWITCH | IS_ACTIVE, NULL, NULL, &amp[0]},
        {"r detect", "rear sound detection", TYPE_PSWITCH | IS_ACTIVE, NULL, NULL, &amp[1]},
        {"f defaul", "front default", TYPE_PSWITCH | IS_ACTIVE, NULL, NULL, &amp[2]},
        {"r defaul", "rear default", TYPE_PSWITCH | IS_ACTIVE, NULL, NULL, &amp[3]},
        {"f cur st", "front current status", TYPE_PSWITCH, NULL, NULL, &amp[4]},
        {"r cur st", "rear current status", TYPE_PSWITCH, NULL, NULL, &amp[5]},
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

	refocus_control();
}

