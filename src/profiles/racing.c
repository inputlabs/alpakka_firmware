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

void config_profile_default_racing(CtrlProfile *profile){
    // Metadata.
    profile->sections[SECTION_META].meta = (CtrlProfileMeta){
        .name="Racing",
        .control_byte = NVM_CONTROL_BYTE,
        .version_major = NVM_PROFILE_VERSION / 1000000,
        .version_minor = (NVM_PROFILE_VERSION / 1000) % 1000,
        .version_patch = NVM_PROFILE_VERSION % 1000,
    };

    // ABXY.
    profile->sections[SECTION_A].button = (CtrlButton){
        .mode=NORMAL,
        .actions={GAMEPAD_A},
        .hint="MDF page",
    };
    profile->sections[SECTION_B].button = (CtrlButton){
        .mode=NORMAL,
        .actions={GAMEPAD_B},
        .hint="Horn",
    };
    profile->sections[SECTION_X].button = (CtrlButton){
        .mode=NORMAL,
        .actions={GAMEPAD_X},
        .hint="Wipers",
    };
    profile->sections[SECTION_Y].button = (CtrlButton){
        .mode=NORMAL,
        .actions={GAMEPAD_Y},
        .hint="Lights",
    };

    // DPad.
    profile->sections[SECTION_DPAD_LEFT].button = (CtrlButton){
        .mode=NORMAL,
        .actions={GAMEPAD_LEFT},
        .hint="MDF -",
    };
    profile->sections[SECTION_DPAD_RIGHT].button = (CtrlButton){
        .mode=NORMAL,
        .actions={GAMEPAD_RIGHT},
        .hint="MDF +",
    };
    profile->sections[SECTION_DPAD_UP].button = (CtrlButton){
        .mode=NORMAL,
        .actions={GAMEPAD_UP},
        .hint="MDF Up",
    };
    profile->sections[SECTION_DPAD_DOWN].button = (CtrlButton){
        .mode=NORMAL,
        .actions={GAMEPAD_DOWN},
        .hint="MDF Down",
    };

    // Select/Start.
    profile->sections[SECTION_SELECT_1].button = (CtrlButton){
        .mode=NORMAL,
        .actions={GAMEPAD_SELECT},
        .hint="View",
    };
    profile->sections[SECTION_START_1].button = (CtrlButton){
        .mode=NORMAL,
        .actions={GAMEPAD_START},
        .hint="Pause",
    };
    profile->sections[SECTION_SELECT_2].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_M},
        .hint="Map",
    };
    profile->sections[SECTION_START_2].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_N},
        .hint="Neutral",
    };

    // Triggers.
    profile->sections[SECTION_L1].button = (CtrlButton){
        .mode=NORMAL,
        .actions={GAMEPAD_L1},
        .hint="Clutch",
    };
    profile->sections[SECTION_R1].button = (CtrlButton){
        .mode=NORMAL,
        .actions={GAMEPAD_R1},
        .hint="Pit limiter",
    };
    profile->sections[SECTION_L2].button = (CtrlButton){
        .mode=NORMAL,
        .actions={GAMEPAD_L3},
        .hint="Gear down",
    };
    profile->sections[SECTION_R2].button = (CtrlButton){
        .mode=NORMAL,
        .actions={GAMEPAD_R3},
        .hint="Gear up",
    };
    profile->sections[SECTION_L4].button = (CtrlButton){
        .mode=NORMAL,
        .actions={GAMEPAD_LEFT},
        .hint="MDF-",
    };
    profile->sections[SECTION_R4].button = (CtrlButton){
        .mode=NORMAL,
        .actions={GAMEPAD_RIGHT},
        .hint="MDF+",
    };

    // Thumbstick (left).
    profile->sections[SECTION_LSTICK_SETTINGS].thumbstick = (CtrlThumbstick){
        .mode=THUMBSTICK_MODE_4DIR,
        .distance_mode=THUMBSTICK_DISTANCE_RADIAL,
        .saturation=100,
        .overlap=80,
    };
    profile->sections[SECTION_LSTICK_UP].button = (CtrlButton){
        .mode=NORMAL,
        .actions={GAMEPAD_AXIS_RZ},
    };
    profile->sections[SECTION_LSTICK_DOWN].button = (CtrlButton){
        .mode=NORMAL,
        .actions={GAMEPAD_AXIS_LZ},
    };
    profile->sections[SECTION_LSTICK_PUSH].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_L},
    };
    profile->sections[SECTION_LSTICK_LEFT].button = (CtrlButton){};
    profile->sections[SECTION_LSTICK_RIGHT].button = (CtrlButton){};
    profile->sections[SECTION_LSTICK_INNER].button = (CtrlButton){};
    profile->sections[SECTION_LSTICK_OUTER].button = (CtrlButton){};

    // Thumbstick (right) / DHat.
    profile->sections[SECTION_RSTICK_SETTINGS].thumbstick = (CtrlThumbstick){
        .mode=THUMBSTICK_MODE_4DIR,
        .saturation=70,
        .overlap=50,
        .deadzone_override=true,
        .deadzone=15,
    };
    profile->sections[SECTION_RSTICK_LEFT].button = (CtrlButton){
        .mode=NORMAL,
        .actions={GAMEPAD_AXIS_RX_NEG, PROC_ROTARY_MODE_1},
    };
    profile->sections[SECTION_RSTICK_RIGHT].button = (CtrlButton){
        .mode=NORMAL,
        .actions={GAMEPAD_AXIS_RX, PROC_ROTARY_MODE_3},
    };
    profile->sections[SECTION_RSTICK_UP].button = (CtrlButton){
        .mode=NORMAL,
        .actions={GAMEPAD_AXIS_RY_NEG, PROC_ROTARY_MODE_4},
    };
    profile->sections[SECTION_RSTICK_DOWN].button = (CtrlButton){
        .mode=NORMAL,
        .actions={GAMEPAD_AXIS_RY, PROC_ROTARY_MODE_2},
    };
    profile->sections[SECTION_RSTICK_PUSH].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_R, PROC_ROTARY_MODE_0},
    };

    // Rotary.
    profile->sections[SECTION_ROTARY_UP].rotary = (CtrlRotary){
        .actions_0={GAMEPAD_UP},
        .actions_1={KEY_PAD_1},
        .actions_2={KEY_PAD_3},
        .actions_3={KEY_PAD_5},
        .actions_4={KEY_PAD_7},
        .hint_0="MDF Up",
        .hint_1="TC+",
        .hint_2="ABS+",
        .hint_3="BB+",
        .hint_4="MIX+",
    };
    profile->sections[SECTION_ROTARY_DOWN].rotary = (CtrlRotary){
        .actions_0={GAMEPAD_DOWN},
        .actions_1={KEY_PAD_2},
        .actions_2={KEY_PAD_4},
        .actions_3={KEY_PAD_6},
        .actions_4={KEY_PAD_8},
        .hint_0="MDF Down",
        .hint_1="TC-",
        .hint_2="ABS-",
        .hint_3="BB-",
        .hint_4="MIX-",
    };

    // Gyro.
    profile->sections[SECTION_GYRO_SETTINGS].gyro = (CtrlGyro){
        .mode=GYRO_MODE_AXIS_ABSOLUTE,
    };
    profile->sections[SECTION_GYRO_X].gyro_axis = (CtrlGyroAxis){
        .actions_neg={GAMEPAD_AXIS_LX_NEG},
        .actions_pos={GAMEPAD_AXIS_LX},
        .angle_min=-90,
        .angle_max=90,
        .hint_pos="Steering",
    };
}
