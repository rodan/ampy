
#ifndef __PGA2311_H__
#define __PGA2311_H__

/// mute/unmute the pga chip identified by 'pga_id'
/// 'save' defines if the new status is saved to the 
///    mixer_settings_t struct
/// inputs:
///   pga_id: [1-6]
///   mute: MUTE or UNMUTE
///   save: 0 - dont save, 1 - save
void pga_set_mute_st(const uint8_t pga_id, const uint8_t mute, const uint8_t save);

/// set the volume for both channels of the pga 
/// identified by 'pga_id'
/// 'save' defines if the new status is saved to the 
///    mixer_settings_t struct
/// if 'autounmute' is true then the pga will be automatically
///   unmuted if volumes > 0
/// inputs:
///   pga_id: [1-6]
///   vol_right: [0-255]
///   vol_left: [0-255]
///   save: 0 - dont save, 1 - save
///   autounmute: 0 - dont unmute, 1 - do unmute if needed
void pga_set_volume(const uint8_t pga_id, const uint8_t vol_right,
                const uint8_t vol_left, const uint8_t save,
                const uint8_t autounmute);

#endif

