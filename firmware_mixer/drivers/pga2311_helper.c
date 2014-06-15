
//
//  author:          Petre Rodan <petre.rodan@simplex.ro>
//  available from:  https://github.com/rodan/ampy
//  license:         GNU GPLv3

#include <stdint.h>
#include "pga2311_helper.h"

/// read out the volume for channel 'ch' on pga 'pga_id' from the 
/// mixer_settings_t structure
/// inputs:
///   pga_id: [1-6]
///   ch:  CH_RIGHT or CH_LEFT
/// returns the volume [0-255]
uint8_t mixer_get_vol_struct(const uint8_t pga_id, const uint8_t ch)
{
    uint8_t *ptr;
    uint8_t rv=0;
    ptr = (uint8_t *) &s.v1_r;

    switch (ch) {

    case CH_RIGHT:
        rv = *(ptr + (pga_id * 2 - 2));
    break;

    case CH_LEFT:
        rv = *(ptr + (pga_id * 2 - 2) + 1);
    break;
    }

    return rv;
}

/// read out the mute status on pga 'pga_id' from the
/// mixer_settings_t structure
/// inputs:
///   pga_id: [1-6]
/// returns the mute status: 1 - pga muted, 0 - pga not muted
uint8_t mixer_get_mute_struct(const uint8_t pga)
{
    return (s.mute_flag & (1 << (pga - 1)));
}

/// set the volume for channel 'ch' on pga 'pga_id' in the 
/// mixer_settings_t structure
/// inputs:
///   pga_id: [1-6]
///   ch:  CH_RIGHT, CH_LEFT or CH_BOTH
/// returns the volume [0-255]
void mixer_set_vol_struct(const uint8_t pga, const uint8_t channel, const uint8_t vol)
{
    uint8_t *ptr;
    ptr = (uint8_t *) &s.v1_r;

    switch (channel) {

    case CH_RIGHT:
        *(ptr + (pga * 2 - 2)) = vol;
    break;

    case CH_LEFT:
        *(ptr + (pga * 2 - 2) + 1) = vol;
    break;

    case CH_BOTH:
        *(ptr + (pga * 2 - 2)) = vol;
        *(ptr + (pga * 2 - 2) + 1) = vol;
    break;

    }
}

/// write the mute status on pga 'pga_id' in the
/// mixer_settings_t structure
/// inputs:
///   pga_id: [1-6]
///   type: MUTE or UNMUTE
/// returns the mute status: 1 - pga muted, 0 - pga not muted
void mixer_set_mute_struct(const uint8_t pga_id, uint8_t type)
{
    switch (type) {

    case MUTE:
        s.mute_flag &= ~(1 << (pga_id - 1));
    break;

    case UNMUTE:
        s.mute_flag |= (1 << (pga_id - 1));
    break;
    }
}


