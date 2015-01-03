
//
//  author:          Petre Rodan <petre.rodan@simplex.ro>
//  available from:  https://github.com/rodan/ampy
//  license:         GNU GPLv3

#include <stdint.h>
#include "pga2311_helper.h"

/// read out the volume for channel 'ch' on pga 'pga' from the 
/// mixer_settings_t structure
/// inputs:
///   pga: [1-6]
///   ch:  CH_RIGHT or CH_LEFT
/// returns the volume [0-255]
uint8_t mixer_get_vol_struct(const uint8_t pga, const uint8_t ch)
{
    uint8_t *ptr;
    uint8_t rv=0;
    ptr = (uint8_t *) &s;

    switch (ch) {

    case CH_RIGHT:
        rv = *(ptr + (pga * 2));
    break;

    case CH_LEFT:
        rv = *(ptr + (pga * 2) + 1);
    break;
    }

    return rv;
}

/// set the volume for channel 'ch' on pga 'pga' in the 
/// mixer_settings_t structure
/// inputs:
///   pga: [1-6]
///   ch:  CH_RIGHT, CH_LEFT or CH_BOTH
/// returns the volume [0-255]
void mixer_set_vol_struct(const uint8_t pga, const uint8_t channel, const uint8_t vol)
{
    uint8_t *ptr;
    ptr = (uint8_t *) &s;

    switch (channel) {

    case CH_RIGHT:
        *(ptr + (pga * 2)) = vol;
    break;

    case CH_LEFT:
        *(ptr + (pga * 2) + 1) = vol;
    break;

    case CH_BOTH:
        *(ptr + (pga * 2)) = vol;
        *(ptr + (pga * 2) + 1) = vol;
    break;

    }
}

/// read out the mute status on pga 'pga' from the
/// mixer_settings_t structure
/// inputs:
///   pga: [1-6]
/// returns the mute status
uint8_t mixer_get_mute_struct(const uint8_t pga)
{
    uint8_t mask;

    mask = 1 << (pga - 1);

    if (s.mute_flag & mask) {
        return UNMUTE;
    }
    return MUTE;
}

/// write the mute status on pga 'pga' in the
/// mixer_settings_t structure
/// inputs:
///   pga: [1-6]
///   type: MUTE or UNMUTE
/// returns the mute status: 1 - pga muted, 0 - pga not muted
void mixer_set_mute_struct(const uint8_t pga, const uint8_t type)
{
    uint8_t mask;

    mask = 1 << (pga - 1);

    switch (type) {

    case MUTE:
        s.mute_flag &= ~mask;
    break;

    case UNMUTE:
        s.mute_flag |= mask;
    break;
    }
}

