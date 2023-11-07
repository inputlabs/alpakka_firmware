// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include "hid.h"
#include "pin.h"
#include "webusb.h"
#include "button.h"

void config_profile_default_home(CtrlProfile *profile){
    // Profile name.
    profile->sections[SECTION_NAME].name = (CtrlProfileName){.name="Home"};

    // ABXY.
    profile->sections[SECTION_A].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_ENTER},
    };
    profile->sections[SECTION_B].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_ESCAPE},
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
        .actions_secondary={KEY_LEFT_CONTROL, KEY_LEFT_ALT, KEY_DELETE},
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
        .actions={KEY_LEFT_SUPER, KEY_D},
        .hint="Show desktop",
    };
    profile->sections[SECTION_R1].button = (CtrlButton){
        .mode=STICKY,
        .actions={KEY_LEFT_ALT},
        .actions_secondary={KEY_TAB},
        .hint="Switch app",
    };
    profile->sections[SECTION_L2].button = (CtrlButton){
        .mode=NORMAL,
        .actions={MOUSE_2},
    };
    profile->sections[SECTION_R2].button = (CtrlButton){
        .mode=NORMAL,
        .actions={MOUSE_1},
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
        .actions_secondary={PROC_TUNE_SENSITIVITY},
    };
    profile->sections[SECTION_DHAT_RIGHT].button = (CtrlButton){
        .mode=HOLD_EXCLUSIVE_LONG,
        .actions={KEY_F3},
        .actions_secondary={PROC_TUNE_TOUCH_THRESHOLD},
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
}