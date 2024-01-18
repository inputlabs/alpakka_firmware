// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include "hid.h"
#include "pin.h"
#include "webusb.h"
#include "button.h"
#include "thumbstick.h"
#include "gyro.h"

void config_profile_default_fps_fusion(CtrlProfile *profile){
    // Profile name.
    profile->sections[SECTION_NAME].name = (CtrlProfileName){.name="FPS Fusion"};

    // ABXY.
    profile->sections[SECTION_A].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_F},
        .hint="Use",
    };
    profile->sections[SECTION_B].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_V},
        .hint="Melee",
    };
    profile->sections[SECTION_X].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_R},
        .hint="Reload",
    };
    profile->sections[SECTION_Y].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_T},
        .hint="Swap",
    };

    // DPad.
    profile->sections[SECTION_DPAD_LEFT].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_LEFT},
        .hint="Equip",
    };
    profile->sections[SECTION_DPAD_RIGHT].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_RIGHT},
        .hint="Equip",
    };
    profile->sections[SECTION_DPAD_UP].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_UP},
        .hint="Equip",
    };
    profile->sections[SECTION_DPAD_DOWN].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_DOWN},
        .hint="Equip",
    };

    // Select/Start.
    profile->sections[SECTION_SELECT_1].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_TAB},
        .hint="Inventory",
    };
    profile->sections[SECTION_START_1].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_ESCAPE},
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
        .hint="Voice",
    };

    // Triggers.
    profile->sections[SECTION_L1].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_Q},
        .hint="Skill",
    };
    profile->sections[SECTION_R1].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_E},
        .hint="Skill",
    };
    profile->sections[SECTION_L2].button = (CtrlButton){
        .mode=NORMAL,
        .actions={MOUSE_2},
        .hint="Aim",
    };
    profile->sections[SECTION_R2].button = (CtrlButton){
        .mode=NORMAL,
        .actions={MOUSE_1},
        .hint="Shoot",
    };
    profile->sections[SECTION_L4].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_SPACE},
        .hint="Jump",
    };
    profile->sections[SECTION_R4].button = (CtrlButton){
        .mode=HOLD_OVERLAP,
        .actions={KEY_CONTROL_LEFT},
        .hint="Crouch",
        .actions_secondary={KEY_P},
        .hint_secondary="Prone",
    };

    // DHat.
    profile->sections[SECTION_DHAT_LEFT].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_1},
    };
    profile->sections[SECTION_DHAT_RIGHT].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_3},
    };
    profile->sections[SECTION_DHAT_UP].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_4},
    };
    profile->sections[SECTION_DHAT_DOWN].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_2},
    };
    profile->sections[SECTION_DHAT_UL].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_8},
    };
    profile->sections[SECTION_DHAT_UR].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_7},
    };
    profile->sections[SECTION_DHAT_DL].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_5},
    };
    profile->sections[SECTION_DHAT_DR].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_6},
    };
    profile->sections[SECTION_DHAT_PUSH].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_0},
        .hint="Weapons",
    };

    // Rotary.
    profile->sections[SECTION_ROTARY_UP].rotary = (CtrlRotary){
        .actions_0={MOUSE_SCROLL_UP},
        .hint_0="Prev / Zoom+",
    };
    profile->sections[SECTION_ROTARY_DOWN].rotary = (CtrlRotary){
        .actions_0={MOUSE_SCROLL_DOWN},
        .hint_0="Next / Zoom-",
    };

    // Thumbstick.
    profile->sections[SECTION_THUMBSTICK].thumbstick = (CtrlThumbstick){
        .mode=THUMBSTICK_MODE_4DIR,
        .overlap=50,
    };
    profile->sections[SECTION_THUMBSTICK_LEFT].button = (CtrlButton){
        .mode=NORMAL,
        .actions={GAMEPAD_AXIS_LX_NEG},
    };
    profile->sections[SECTION_THUMBSTICK_RIGHT].button = (CtrlButton){
        .mode=NORMAL,
        .actions={GAMEPAD_AXIS_LX},
    };
    profile->sections[SECTION_THUMBSTICK_UP].button = (CtrlButton){
        .mode=NORMAL,
        .actions={GAMEPAD_AXIS_LY_NEG},
    };
    profile->sections[SECTION_THUMBSTICK_DOWN].button = (CtrlButton){
        .mode=NORMAL,
        .actions={GAMEPAD_AXIS_LY},
    };
    profile->sections[SECTION_THUMBSTICK_PUSH].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_SHIFT_LEFT},
        .hint="Sprint",
    };
    profile->sections[SECTION_THUMBSTICK_INNER].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_BRACKET_LEFT},
        .hint="Walk",
    };
    profile->sections[SECTION_THUMBSTICK_OUTER].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_BRACKET_RIGHT},
        .hint="Run",
    };

    // Gyro.
    profile->sections[SECTION_GYRO].gyro = (CtrlGyro){
        .mode=GYRO_MODE_TOUCH_ON,
        .engage=PIN_TOUCH_IN,
    };
    profile->sections[SECTION_GYRO_X].gyro_axis = (CtrlGyroAxis){
        .actions_neg={MOUSE_X_NEG},
        .actions_pos={MOUSE_X},
        .hint_pos="Mouse",
    };
    profile->sections[SECTION_GYRO_Y].gyro_axis = (CtrlGyroAxis){
        .actions_neg={MOUSE_Y_NEG},
        .actions_pos={MOUSE_Y},
        .hint_pos="Mouse",
    };
}
