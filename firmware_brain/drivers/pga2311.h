#ifndef __PGA2311_H__
#define __PGA2311_H__

struct mixer_settings_t {
    uint8_t ver;                // firmware version
    uint8_t mute_flag;
    uint8_t v1_r;               // pga #1, right channel volume
    uint8_t v1_l;               // pga #1, left channel volume
    uint8_t v2_r;               // ...
    uint8_t v2_l;
    uint8_t v3_r;
    uint8_t v3_l;
    uint8_t v4_r;
    uint8_t v4_l;
    uint8_t v5_r;
    uint8_t v5_l;
    uint8_t v6_r;
    uint8_t v6_l;               // pga #6, left channel volume
};

struct mixer_settings_t s;

#define VOL_STEP        2
#define FCT_T_MUTE      0x1
#define FCT_V_INC       0x2
#define FCT_V_DEC       0x3

#define CH_RIGHT        0x0
#define CH_LEFT         0x1
#define CH_BOTH         0x2

#define MUTE            0x0
#define UNMUTE          0x1

/// read out the volume for channel 'ch' on pga 'pga_id' from the 
/// mixer_settings_t structure
/// inputs:
///   pga_id: [1-6]
///   ch:  CH_RIGHT or CH_LEFT
/// returns the volume [0-255]
uint8_t mixer_get_vol_struct(const uint8_t pga_id, const uint8_t ch);

/// read out the mute status on pga 'pga_id' from the
/// mixer_settings_t structure
/// inputs:
///   pga_id: [1-6]
/// returns the mute status: 1 - pga muted, 0 - pga not muted
uint8_t mixer_get_mute_struct(const uint8_t pga);

/// set the volume for channel 'ch' on pga 'pga_id' in the 
/// mixer_settings_t structure
/// inputs:
///   pga_id: [1-6]
///   ch:  CH_RIGHT, CH_LEFT or CH_BOTH
/// returns the volume [0-255]
void mixer_set_vol_struct(const uint8_t pga, const uint8_t channel, const uint8_t vol);

/// write the mute status on pga 'pga_id' in the
/// mixer_settings_t structure
/// inputs:
///   pga_id: [1-6]
///   type: MUTE or UNMUTE
/// returns the mute status: 1 - pga muted, 0 - pga not muted
void mixer_set_mute_struct(const uint8_t pga_id, uint8_t type);


#endif
