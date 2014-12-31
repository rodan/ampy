
// LM4780 has two pins that can mute out the output if the applied 
// voltage is switched from the negative rail voltage (Vss) to GND
// 

#include "lm4780_helper.h"

// structure that holds the status registers for up to 8 lm4780s.

/// read out the sound detection setting of channel from the
/// ampy_settings_t structure
/// inputs:
///   channel: [1-8]
/// returns the snd detection setting
uint8_t ampy_get_detect(const uint8_t channel)
{
    uint8_t mask;

    mask = 1 << (channel - 1);

    if (a.snd_detect & mask) {
        return 1;
    }
    return 0;
}

/// write the mute status on channel into the
/// ampy_settings_t structure
/// inputs:
///   channel: [1-8]
///   type: MUTE or UNMUTE
void ampy_set_mute(const uint8_t channel, const uint8_t type)
{
    uint8_t mask;

    mask = 1 << (channel - 1);

    switch (type) {

    case MUTE:
        a.mute_flag &= ~mask;
    break;

    case UNMUTE:
        a.mute_flag |= mask;
    break;
    }
}

/// read out the mute status of channel from the
/// ampy_settings_t structure
/// inputs:
///   channel: [1-8]
/// returns the mute status
uint8_t ampy_get_mute(const uint8_t channel)
{
    uint8_t mask;

    mask = 1 << (channel - 1);

    if (a.mute_flag & mask) {
        return UNMUTE;
    }
    return MUTE;
}

/// write the mute status on channel into the
/// ampy_settings_t structure
/// inputs:
///   channel: [1-8]
///   type: MUTE or UNMUTE
void ampy_set_status(const uint8_t channel, const uint8_t type)
{
    uint8_t mask;

    mask = 1 << (channel - 1);

    switch (type) {

    case MUTE:
        a.mute_status &= ~mask;
    break;

    case UNMUTE:
        a.mute_status |= mask;
    break;
    }
}

/// read out the mute status of channel from the
/// ampy_settings_t structure
/// inputs:
///   channel: [1-8]
/// returns the mute status
uint8_t ampy_get_status(const uint8_t channel)
{
    uint8_t mask;

    mask = 1 << (channel - 1);

    if (a.mute_status & mask) {
        return UNMUTE;
    }
    return MUTE;
}

