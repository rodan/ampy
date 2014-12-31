#ifndef __LM4780_HELPER_H__
#define __LM4780_HELPER_H__

#include <inttypes.h>

#define A_VER   0x1

struct ampy_settings_t {
    uint8_t ver;            // version stamp for the structure (it ends up in eeprom)
    uint8_t snd_detect;     // bitwise representation for 8 channels of sound detection 
    uint8_t mute_flag;      // mute settings when sound detection is disabled 
    uint8_t mute_status;    // current mute statuses
};

struct ampy_settings_t a;

#ifndef MUTE
#define MUTE 0x0
#endif

#ifndef LIVE
#define LIVE 0x1
#endif

/// write the detect setting on channel into the
/// ampy_settings_t structure
/// inputs:
///   channel: [1-8]
///   type: 1 (true) or 0 (false)
void ampy_set_detect(const uint8_t channel, const uint8_t type);

/// read out the sound detection setting of channel from the
/// ampy_settings_t structure
/// inputs:
///   channel: [1-8]
/// returns the snd detect setting: 1 - enabled, 0 - disabled
uint8_t ampy_get_detect(const uint8_t channel);

/// write the mute status on channel into the
/// ampy_settings_t structure
/// inputs:
///   channel: [1-8]
///   type: MUTE or LIVE
void ampy_set_mute(const uint8_t channel, const uint8_t type);

/// read out the mute setting of channel from the
/// ampy_settings_t structure
/// inputs:
///   channel: [1-8]
/// returns the mute status: MUTE - pga muted, LIVE - pga not muted
uint8_t ampy_get_mute(const uint8_t channel);

/// write the mute status on channel into the
/// ampy_settings_t structure
/// inputs:
///   channel: [1-8]
///   type: MUTE or LIVE
void ampy_set_status(const uint8_t channel, const uint8_t type);

/// read out the mute status of channel from the
/// ampy_settings_t structure
/// inputs:
///   channel: [1-8]
/// returns the mute status: MUTE - pga muted, LIVE - pga not muted
uint8_t ampy_get_status(const uint8_t channel);

#endif
