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

void config_profile_default_home(CtrlProfile profile){
    // ABXY.
    assign(profile, SECTION_A, NORMAL, KEY_ENTER, END);
    assign(profile, SECTION_B, NORMAL, KEY_ESCAPE, END);
    assign(profile, SECTION_X, NORMAL, KEY_BACKSPACE, END);
    assign(profile, SECTION_Y, NORMAL, KEY_SPACE, END);
    // DPad.
    assign(profile, SECTION_DPAD_UP,    HOLD_OVERLAP, ACTIONSX(PROC_PROFILE_1), ACTIONSX(PROC_PROFILE_5), END);
    assign(profile, SECTION_DPAD_RIGHT, HOLD_OVERLAP, ACTIONSX(PROC_PROFILE_2), ACTIONSX(PROC_PROFILE_6), END);
    assign(profile, SECTION_DPAD_DOWN,  HOLD_OVERLAP, ACTIONSX(PROC_PROFILE_3), ACTIONSX(PROC_PROFILE_7), END);
    assign(profile, SECTION_DPAD_LEFT,  HOLD_OVERLAP, ACTIONSX(PROC_PROFILE_4), ACTIONSX(PROC_PROFILE_8), END);
    // Select.
    assign(profile, SECTION_SELECT_1, HOLD_EXCLUSIVE_LONG, ACTIONSX(KEY_F9), ACTIONSX(PROC_BOOTSEL), END);
    assign(profile, SECTION_START_1,  HOLD_EXCLUSIVE_LONG, ACTIONSX(KEY_F10), ACTIONSX(KEY_LEFT_CONTROL, KEY_LEFT_ALT, KEY_DELETE), END);
    assign(profile, SECTION_SELECT_2, NORMAL, KEY_F11, END);
    assign(profile, SECTION_START_2,  HOLD_EXCLUSIVE_LONG, ACTIONSX(KEY_F12), ACTIONSX(PROC_CALIBRATE), END);
    // Triggers.
    assign(profile, SECTION_L1, NORMAL, KEY_LEFT_SUPER, KEY_D, END);
    assign(profile, SECTION_R1, STICKY, ACTIONSX(KEY_LEFT_ALT), ACTIONSX(KEY_TAB), END);
    assign(profile, SECTION_L2, NORMAL, MOUSE_2, END);
    assign(profile, SECTION_R2, NORMAL, MOUSE_1, END);
    assign(profile, SECTION_L4, NORMAL, KEY_NONE, END);
    assign(profile, SECTION_R4, NORMAL, KEY_NONE, END);
    // DHat.
    assign(profile, SECTION_DHAT_LEFT,  HOLD_EXCLUSIVE_LONG, ACTIONSX(KEY_F1), ACTIONSX(PROC_TUNE_DEADZONE), END);
    assign(profile, SECTION_DHAT_DOWN,  HOLD_EXCLUSIVE_LONG, ACTIONSX(KEY_F2), ACTIONSX(PROC_TUNE_SENSITIVITY), END);
    assign(profile, SECTION_DHAT_RIGHT, HOLD_EXCLUSIVE_LONG, ACTIONSX(KEY_F3), ACTIONSX(PROC_TUNE_TOUCH_THRESHOLD), END);
    assign(profile, SECTION_DHAT_UP,    HOLD_EXCLUSIVE_LONG, ACTIONSX(KEY_F4), ACTIONSX(PROC_TUNE_OS), END);
    assign(profile, SECTION_DHAT_UL,    NORMAL, KEY_F8, END);
    assign(profile, SECTION_DHAT_UR,    NORMAL, KEY_F7, END);
    assign(profile, SECTION_DHAT_DL,    NORMAL, KEY_F5, END);
    assign(profile, SECTION_DHAT_DR,    NORMAL, KEY_F6, END);
    assign(profile, SECTION_DHAT_PUSH,  NORMAL, KEY_BACKQUOTE, END);
    // Strings.
    string(profile, SECTION_NAME, 0, "Home");
    string(profile, SECTION_L1, BUTTON_HINT, "Show desktop");
    string(profile, SECTION_R1, BUTTON_HINT_SECONDARY, "Switch app");
}
