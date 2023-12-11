// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include "hid.h"
#include "pin.h"
#include "webusb.h"
#include "button.h"
#include "thumbstick.h"
#include "gyro.h"

void config_profile_default_racing(CtrlProfile *profile){
    // Profile name.
    profile->sections[SECTION_NAME].name = (CtrlProfileName){.name="Racing"};

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
        .mode=HOLD_OVERLAP,
        .actions={GAMEPAD_RIGHT},
        .hint="MDF+",
    };

    // DHat.
    profile->sections[SECTION_DHAT_LEFT].button = (CtrlButton){
        .mode=NORMAL,
        .actions={GAMEPAD_AXIS_RX_NEG, PROC_ROTARY_MODE_1},
    };
    profile->sections[SECTION_DHAT_RIGHT].button = (CtrlButton){
        .mode=NORMAL,
        .actions={GAMEPAD_AXIS_RX, PROC_ROTARY_MODE_3},
    };
    profile->sections[SECTION_DHAT_UP].button = (CtrlButton){
        .mode=NORMAL,
        .actions={GAMEPAD_AXIS_RY_NEG, PROC_ROTARY_MODE_4},
    };
    profile->sections[SECTION_DHAT_DOWN].button = (CtrlButton){
        .mode=NORMAL,
        .actions={GAMEPAD_AXIS_RY, PROC_ROTARY_MODE_2},
    };
    profile->sections[SECTION_DHAT_UL].button = (CtrlButton){
        .mode=NORMAL,
        .actions={GAMEPAD_AXIS_RX_NEG, GAMEPAD_AXIS_RY_NEG},
    };
    profile->sections[SECTION_DHAT_UR].button = (CtrlButton){
        .mode=NORMAL,
        .actions={GAMEPAD_AXIS_RX, GAMEPAD_AXIS_RY_NEG},
    };
    profile->sections[SECTION_DHAT_DL].button = (CtrlButton){
        .mode=NORMAL,
        .actions={GAMEPAD_AXIS_RX_NEG, GAMEPAD_AXIS_RY},
    };
    profile->sections[SECTION_DHAT_DR].button = (CtrlButton){
        .mode=NORMAL,
        .actions={GAMEPAD_AXIS_RX, GAMEPAD_AXIS_RY},
    };
    profile->sections[SECTION_DHAT_PUSH].button = (CtrlButton){
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

    // Thumbstick.
    profile->sections[SECTION_THUMBSTICK].thumbstick = (CtrlThumbstick){
        .mode=THUMBSTICK_MODE_4DIR,
        .distance_mode=THUMBSTICK_DISTANCE_RADIAL,
        .deadzone=DEADZONE_FROM_CONFIG,
        .overlap=80,
    };
    profile->sections[SECTION_THUMBSTICK_UP].button = (CtrlButton){
        .mode=NORMAL,
        .actions={GAMEPAD_AXIS_RZ},
    };
    profile->sections[SECTION_THUMBSTICK_DOWN].button = (CtrlButton){
        .mode=NORMAL,
        .actions={GAMEPAD_AXIS_LZ},
    };
    profile->sections[SECTION_THUMBSTICK_PUSH].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_L},
    };
    profile->sections[SECTION_THUMBSTICK_LEFT].button = (CtrlButton){};
    profile->sections[SECTION_THUMBSTICK_RIGHT].button = (CtrlButton){};
    profile->sections[SECTION_THUMBSTICK_INNER].button = (CtrlButton){};
    profile->sections[SECTION_THUMBSTICK_OUTER].button = (CtrlButton){};

    // Gyro.
    profile->sections[SECTION_GYRO].gyro = (CtrlGyro){
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
