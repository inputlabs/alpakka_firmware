// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include "hid.h"
#include "pin.h"
#include "webusb.h"
#include "button.h"
#include "thumbstick.h"
#include "gyro.h"

void config_profile_default_home(CtrlProfile *profile){
    // Profile name.
    profile->sections[SECTION_NAME].name = (CtrlProfileName){.name="Home"};

    // ABXY.
    profile->sections[SECTION_A].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_ENTER},
        .hint="Accept",
    };
    profile->sections[SECTION_B].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_ESCAPE},
        .hint="Cancel",
    };
    profile->sections[SECTION_X].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_BACKSPACE},
    };
    profile->sections[SECTION_Y].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_SPACE},
    };

    // DPad.
    profile->sections[SECTION_DPAD_UP].button = (CtrlButton){
        .mode=HOLD_OVERLAP,
        .actions={PROC_PROFILE_1},
        .actions_secondary={PROC_PROFILE_5},
    };
    profile->sections[SECTION_DPAD_RIGHT].button = (CtrlButton){
        .mode=HOLD_OVERLAP,
        .actions={PROC_PROFILE_2},
        .actions_secondary={PROC_PROFILE_6},
    };
    profile->sections[SECTION_DPAD_DOWN].button = (CtrlButton){
        .mode=HOLD_OVERLAP,
        .actions={PROC_PROFILE_3},
        .actions_secondary={PROC_PROFILE_7},
    };
    profile->sections[SECTION_DPAD_LEFT].button = (CtrlButton){
        .mode=HOLD_OVERLAP,
        .actions={PROC_PROFILE_4},
        .actions_secondary={PROC_PROFILE_8},
    };

    // Select.
    profile->sections[SECTION_SELECT_1].button = (CtrlButton){
        .mode=HOLD_EXCLUSIVE_LONG,
        .actions={KEY_F9},
        .actions_secondary={PROC_BOOTSEL},
    };
    profile->sections[SECTION_START_1].button = (CtrlButton){
        .mode=HOLD_EXCLUSIVE_LONG,
        .actions={KEY_F10},
        .actions_secondary={KEY_CONTROL_LEFT, KEY_ALT_LEFT, KEY_DELETE},
    };
    profile->sections[SECTION_SELECT_2].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_F11},
    };
    profile->sections[SECTION_START_2].button = (CtrlButton){
        .mode=HOLD_EXCLUSIVE_LONG,
        .actions={KEY_F12},
        .actions_secondary={PROC_CALIBRATE},
    };

    // Triggers.
    profile->sections[SECTION_L1].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_SUPER_LEFT, KEY_D},
        .hint="Show desktop",
    };
    profile->sections[SECTION_R1].button = (CtrlButton){
        .mode=STICKY,
        .actions={KEY_ALT_LEFT},
        .actions_secondary={KEY_TAB},
        .hint_secondary="Switch app",
    };
    profile->sections[SECTION_L2].button = (CtrlButton){
        .mode=NORMAL,
        .actions={MOUSE_2},
        .hint="Click",
    };
    profile->sections[SECTION_R2].button = (CtrlButton){
        .mode=NORMAL,
        .actions={MOUSE_1},
        .hint="Click",
    };
    profile->sections[SECTION_L4].button = (CtrlButton){};
    profile->sections[SECTION_R4].button = (CtrlButton){};

    // DHat.
    profile->sections[SECTION_DHAT_LEFT].button = (CtrlButton){
        .mode=HOLD_EXCLUSIVE_LONG,
        .actions={KEY_F1},
        .actions_secondary={PROC_TUNE_DEADZONE},
    };
    profile->sections[SECTION_DHAT_DOWN].button = (CtrlButton){
        .mode=HOLD_EXCLUSIVE_LONG,
        .actions={KEY_F2},
        .actions_secondary={PROC_TUNE_MOUSE_SENS},
    };
    profile->sections[SECTION_DHAT_RIGHT].button = (CtrlButton){
        .mode=HOLD_EXCLUSIVE_LONG,
        .actions={KEY_F3},
        .actions_secondary={PROC_TUNE_TOUCH_SENS},
    };
    profile->sections[SECTION_DHAT_UP].button = (CtrlButton){
        .mode=HOLD_EXCLUSIVE_LONG,
        .actions={KEY_F4},
        .actions_secondary={PROC_TUNE_OS},
    };
    profile->sections[SECTION_DHAT_UL].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_F8},
    };
    profile->sections[SECTION_DHAT_UR].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_F7},
    };
    profile->sections[SECTION_DHAT_DL].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_F5}
    };
    profile->sections[SECTION_DHAT_DR].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_F6}
    };
    profile->sections[SECTION_DHAT_PUSH].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_BACKQUOTE},
    };

    // Rotary.
    profile->sections[SECTION_ROTARY_UP].rotary = (CtrlRotary){
        .actions_0={PROC_TUNE_UP},
    };
    profile->sections[SECTION_ROTARY_DOWN].rotary = (CtrlRotary){
        .actions_0={PROC_TUNE_DOWN},
    };

    // Thumbstick.
    profile->sections[SECTION_THUMBSTICK].thumbstick = (CtrlThumbstick){
        .mode=THUMBSTICK_MODE_4DIR,
        .deadzone_override=true,
        .deadzone=25,  // To avoid quickchat accidental triggers.
        .overlap=-50,  // Ignore diagonals because quickchat.
    };
    profile->sections[SECTION_THUMBSTICK_LEFT].button = (CtrlButton){
        .mode=HOLD_EXCLUSIVE,
        .actions={PROC_MACRO_1},
        .actions_secondary={PROC_MACRO_5},
    };
    profile->sections[SECTION_THUMBSTICK_RIGHT].button = (CtrlButton){
        .mode=HOLD_EXCLUSIVE,
        .actions={PROC_MACRO_2},
        .actions_secondary={PROC_MACRO_6},
    };
    profile->sections[SECTION_THUMBSTICK_UP].button = (CtrlButton){
        .mode=HOLD_EXCLUSIVE,
        .actions={PROC_MACRO_3},
        .actions_secondary={PROC_MACRO_7},
    };
    profile->sections[SECTION_THUMBSTICK_DOWN].button = (CtrlButton){
        .mode=HOLD_EXCLUSIVE,
        .actions={PROC_MACRO_4},
        .actions_secondary={PROC_MACRO_8},
    };
    profile->sections[SECTION_THUMBSTICK_PUSH].button = (CtrlButton){
        .mode=HOLD_EXCLUSIVE_LONG,
        .actions={KEY_SUPER_LEFT, KEY_CONTROL_LEFT, KEY_O},
        .hint="On-screen KB",
        .actions_secondary={PROC_THANKS},
    };
    profile->sections[SECTION_THUMBSTICK_INNER].button = (CtrlButton){};
    profile->sections[SECTION_THUMBSTICK_OUTER].button = (CtrlButton){};

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

    // Macros.
    profile->sections[SECTION_MACRO_1].macro = (CtrlMacro){
        .macro={
            {KEY_S, KEY_O, KEY_R, KEY_R, KEY_Y, KEY_SPACE},  // Sorry.
            {KEY_O, KEY_K, KEY_SPACE},                       // Ok.
        }
    };
    profile->sections[SECTION_MACRO_2].macro = (CtrlMacro){
        .macro={
            {KEY_H, KEY_E, KEY_L, KEY_L, KEY_O, KEY_SPACE},  // Hello.
            {KEY_N, KEY_I, KEY_C, KEY_E, KEY_SPACE},         // Nice.
        }
    };
    profile->sections[SECTION_MACRO_3].macro = (CtrlMacro){
        .macro={
            {KEY_N, KEY_O, KEY_SPACE},         // No.
            {KEY_Y, KEY_E, KEY_S, KEY_SPACE},  // Yes.
        }
    };
    profile->sections[SECTION_MACRO_4].macro = (CtrlMacro){
        .macro={
            {KEY_G, KEY_G, KEY_SPACE},                              // gg.
            {KEY_T, KEY_H, KEY_A, KEY_N, KEY_K, KEY_S, KEY_SPACE},  // Thanks.
        }
    };
}
