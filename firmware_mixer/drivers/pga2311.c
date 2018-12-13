
//
//  author:          Petre Rodan <2b4eda@subdimension.ro>
//  available from:  https://github.com/rodan/ampy
//  license:         GNU GPLv3

#include <stdint.h>
#include "pga2311.h"
#include "pga2311_config.h"
#include "pga2311_helper.h"
#include "drivers/spi.h"

/// mute/unmute the pga chip identified by 'pga_id'
/// 'save' defines if the new status is saved to the 
///    mixer_settings_t struct
/// inputs:
///   pga_id: [1-6]
///   mute_st: MUTE or UNMUTE
///   save: 0 - dont save, 1 - save
void pga_set_mute_st(const uint8_t pga_id, const uint8_t mute_st, const uint8_t save)
{
    uint8_t *ptr;

    if (mute_st == MUTE) {
        // set the proper PxOUT port to low in order to mute the PGA out
        ptr = (uint8_t *) MUTE_OUT[pga_id - 1];
        *ptr &= ~MUTE_PORT[pga_id - 1];
        if (save) {
            mixer_set_mute_struct(pga_id, MUTE);
        }
    } else if (mute_st == UNMUTE) {
        // unmute port
        ptr = (uint8_t *) MUTE_OUT[pga_id - 1];
        *ptr |= MUTE_PORT[pga_id - 1];
        if (save) {
            mixer_set_mute_struct(pga_id, UNMUTE);
        }
    }
}

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
                const uint8_t autounmute)
{
    uint8_t data[2];
    uint8_t *ptr;

    if ((pga_id < 1) || (pga_id > 6)) {
        return;
    }

    //if ((vol_left == 0) && (vol_right == 0)) {
    //    pga_mute(pga_id, save);
    //} else {
        data[0] = vol_right;
        data[1] = vol_left;

        if (autounmute) {
            pga_set_mute_st(pga_id, UNMUTE, save);
        }

        if (save) {
            mixer_set_vol_struct(pga_id, CH_RIGHT, vol_right);
            mixer_set_vol_struct(pga_id, CH_LEFT, vol_left);
        }
        // select slave
        ptr = (uint8_t *) CS_OUT[pga_id - 1];
        *ptr &= ~CS_PORT[pga_id - 1];

        // set volume
        spi_send_frame(data, 2);

        // deselect slave
        *ptr |= CS_PORT[pga_id - 1];
    //}
}

