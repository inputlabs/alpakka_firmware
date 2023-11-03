// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include "hid.h"
#include "pin.h"
#include "webusb.h"
#include "profile.h"
#include "button.h"

#define assign profile_assign
#define string profile_string

void config_profile_default_fps_fusion(CtrlProfile profile){
    // ABXY.
    assign(profile, SECTION_A, NORMAL, KEY_F, END);
    assign(profile, SECTION_B, NORMAL, KEY_V, END);
    assign(profile, SECTION_X, NORMAL, KEY_R, END);
    assign(profile, SECTION_Y, NORMAL, KEY_T, END);
    // DPad.
    assign(profile, SECTION_DPAD_LEFT,  NORMAL, KEY_LEFT, END);
    assign(profile, SECTION_DPAD_RIGHT, NORMAL, KEY_RIGHT, END);
    assign(profile, SECTION_DPAD_UP,    NORMAL, KEY_UP, END);
    assign(profile, SECTION_DPAD_DOWN,  NORMAL, KEY_DOWN, END);
    // Select.
    assign(profile, SECTION_SELECT_1, NORMAL, KEY_TAB, END);
    assign(profile, SECTION_START_1,  NORMAL, KEY_ESCAPE, END);
    assign(profile, SECTION_SELECT_2, NORMAL, KEY_M, END);
    assign(profile, SECTION_START_2,  NORMAL, KEY_N, END);
    // Triggers.
    assign(profile, SECTION_L1, NORMAL, KEY_Q, END);
    assign(profile, SECTION_R1, NORMAL, KEY_E, END);
    assign(profile, SECTION_L2, NORMAL, MOUSE_2, END);
    assign(profile, SECTION_R2, NORMAL, MOUSE_1, END);
    assign(profile, SECTION_L4, NORMAL, KEY_SPACE, END);
    assign(profile, SECTION_R4, HOLD_OVERLAP, ACTIONSX(KEY_LEFT_CONTROL), ACTIONSX(KEY_P), END);
    // DHat.
    assign(profile, SECTION_DHAT_LEFT,  NORMAL, KEY_1, END);
    assign(profile, SECTION_DHAT_RIGHT, NORMAL, KEY_3, END);
    assign(profile, SECTION_DHAT_UP,    NORMAL, KEY_4, END);
    assign(profile, SECTION_DHAT_DOWN,  NORMAL, KEY_2, END);
    assign(profile, SECTION_DHAT_UL,    NORMAL, KEY_8, END);
    assign(profile, SECTION_DHAT_UR,    NORMAL, KEY_7, END);
    assign(profile, SECTION_DHAT_DL,    NORMAL, KEY_5, END);
    assign(profile, SECTION_DHAT_DR,    NORMAL, KEY_6, END);
    assign(profile, SECTION_DHAT_PUSH,  NORMAL, KEY_0, END);
    // Strings.
    string(profile, SECTION_NAME, 0, "FPS Fusion");
    string(profile, SECTION_A, BUTTON_HINT, "Use");
    string(profile, SECTION_B, BUTTON_HINT, "Melee");
    string(profile, SECTION_X, BUTTON_HINT, "Reload");
    string(profile, SECTION_Y, BUTTON_HINT, "Swap");
    string(profile, SECTION_L1, BUTTON_HINT, "Skill");
    string(profile, SECTION_R1, BUTTON_HINT, "Skill");
    string(profile, SECTION_L2, BUTTON_HINT, "Aim");
    string(profile, SECTION_R2, BUTTON_HINT, "Shoot");
    string(profile, SECTION_R4, BUTTON_HINT, "Crouch");
    string(profile, SECTION_R4, BUTTON_HINT_SECONDARY, "Prone");
    string(profile, SECTION_SELECT_1, BUTTON_HINT, "Inventory");
    string(profile, SECTION_START_1, BUTTON_HINT, "Pause");
    string(profile, SECTION_SELECT_2, BUTTON_HINT, "Map");
    string(profile, SECTION_START_2, BUTTON_HINT, "Voice");
}
