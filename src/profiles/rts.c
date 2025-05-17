// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include "hid.h"
#include "pin.h"
#include "webusb.h"
#include "button.h"
#include "thumbstick.h"
#include "gyro.h"
#include "config.h"

void config_profile_default_rts(CtrlProfile *profile){
    // Metadata.
    profile->sections[SECTION_META].meta = (CtrlProfileMeta){
        .name="RTS",
        .control_byte = NVM_CONTROL_BYTE,
        .version_major = NVM_PROFILE_VERSION / 1000000,
        .version_minor = (NVM_PROFILE_VERSION / 1000) % 1000,
        .version_patch = NVM_PROFILE_VERSION % 1000,
    };

    // ABXY.
    profile->sections[SECTION_A].button = (CtrlButton){};
    profile->sections[SECTION_B].button = (CtrlButton){};
    profile->sections[SECTION_X].button = (CtrlButton){};
    profile->sections[SECTION_Y].button = (CtrlButton){};

    // DPad.
    profile->sections[SECTION_DPAD_LEFT].button = (CtrlButton){};
    profile->sections[SECTION_DPAD_RIGHT].button = (CtrlButton){};
    profile->sections[SECTION_DPAD_UP].button = (CtrlButton){};
    profile->sections[SECTION_DPAD_DOWN].button = (CtrlButton){};

    // Select/Start.
    profile->sections[SECTION_SELECT_1].button = (CtrlButton){};
    profile->sections[SECTION_START_1].button = (CtrlButton){};
    profile->sections[SECTION_SELECT_2].button = (CtrlButton){};
    profile->sections[SECTION_START_2].button = (CtrlButton){};

    // Triggers.
    profile->sections[SECTION_L1].button = (CtrlButton){};
    profile->sections[SECTION_R1].button = (CtrlButton){};
    profile->sections[SECTION_L2].button = (CtrlButton){};
    profile->sections[SECTION_R2].button = (CtrlButton){};
    profile->sections[SECTION_L4].button = (CtrlButton){};
    profile->sections[SECTION_R4].button = (CtrlButton){};

    // Thumbstick (left).
    profile->sections[SECTION_LSTICK_SETTINGS].thumbstick = (CtrlThumbstick){
        .mode=THUMBSTICK_MODE_OFF,
        .overlap=50,
        .saturation=100,
    };

    // Thumbstick (right) / DHat.
    profile->sections[SECTION_RSTICK_SETTINGS].thumbstick = (CtrlThumbstick){
        .mode=THUMBSTICK_MODE_OFF,
        .overlap=50,
        .saturation=70,
    };
    // profile->sections[SECTION_RSTICK_LEFT].button = (CtrlButton){};
    // profile->sections[SECTION_RSTICK_RIGHT].button = (CtrlButton){};
    // profile->sections[SECTION_RSTICK_UP].button = (CtrlButton){};
    // profile->sections[SECTION_RSTICK_DOWN].button = (CtrlButton){};
    // profile->sections[SECTION_RSTICK_UL].button = (CtrlButton){};
    // profile->sections[SECTION_RSTICK_UR].button = (CtrlButton){};
    // profile->sections[SECTION_RSTICK_DL].button = (CtrlButton){};
    // profile->sections[SECTION_RSTICK_DR].button = (CtrlButton){};
    // profile->sections[SECTION_RSTICK_PUSH].button = (CtrlButton){};

    // Rotary.
    profile->sections[SECTION_ROTARY_UP].rotary = (CtrlRotary){};
    profile->sections[SECTION_ROTARY_DOWN].rotary = (CtrlRotary){};

    // Gyro.
    profile->sections[SECTION_GYRO_SETTINGS].gyro = (CtrlGyro){
        .mode=GYRO_MODE_OFF,
    };
}
