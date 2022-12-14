// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include "pin.h"
#include "hid.h"
#include "button.h"
#include "profile.h"

Profile profile_init_home() {
    Profile profile = Profile_();

    profile.select_1 = Button_(
        PIN_SELECT_1,
        HOLD_EXCLUSIVE_LONG,
        ACTIONS(KEY_F9),
        ACTIONS(PROC_BOOTSEL)
    );
    profile.select_2 = Button_(
        PIN_SELECT_2,
        NORMAL,
        ACTIONS(KEY_F11)
    );
    profile.start_1 = Button_(
        PIN_START_1,
        HOLD_EXCLUSIVE_LONG,
        ACTIONS(KEY_F10),
        ACTIONS(KEY_LEFT_CONTROL, KEY_LEFT_ALT, KEY_DELETE)
    );
    profile.start_2 = Button_(
        PIN_START_2,
        HOLD_EXCLUSIVE_LONG,
        ACTIONS(KEY_F12),
        ACTIONS(PROC_CALIBRATE)
    );

    profile.dpad_left =  Button_( PIN_DPAD_LEFT,  HOLD_OVERLAP, ACTIONS(PROC_PROFILE_4), ACTIONS(PROC_PROFILE_8) );
    profile.dpad_right = Button_( PIN_DPAD_RIGHT, HOLD_OVERLAP, ACTIONS(PROC_PROFILE_2), ACTIONS(PROC_PROFILE_6) );
    profile.dpad_up =    Button_( PIN_DPAD_UP,    HOLD_OVERLAP, ACTIONS(PROC_PROFILE_1), ACTIONS(PROC_PROFILE_5) );
    profile.dpad_down =  Button_( PIN_DPAD_DOWN,  HOLD_OVERLAP, ACTIONS(PROC_PROFILE_3), ACTIONS(PROC_PROFILE_7) );

    profile.a = Button_( PIN_A, NORMAL, ACTIONS(KEY_ENTER) );
    profile.b = Button_( PIN_B, NORMAL, ACTIONS(KEY_ESCAPE) );
    profile.x = Button_( PIN_X, NORMAL, ACTIONS(KEY_BACKSPACE) );
    profile.y = Button_( PIN_Y, NORMAL, ACTIONS(KEY_SPACE) );

    profile.l1 = Button_( PIN_L1, NORMAL, ACTIONS(KEY_LEFT_SUPER, KEY_D) );
    profile.r1 = Button_( PIN_R1, STICKY, ACTIONS(KEY_LEFT_ALT), ACTIONS(KEY_TAB) );
    profile.l2 = Button_( PIN_L2, NORMAL, ACTIONS(MOUSE_2) );
    profile.r2 = Button_( PIN_R2, NORMAL, ACTIONS(MOUSE_1) );
    profile.l4 = Button_( PIN_L4, NORMAL, ACTIONS(KEY_G) );
    profile.r4 = Button_( PIN_R4, NORMAL, ACTIONS(KEY_P) );

    profile.rotary = Rotary_( 0, ACTIONS(PROC_TUNE_UP), ACTIONS(PROC_TUNE_DOWN) );

    profile.thumbstick = Thumbstick_(
        Button_(0, HOLD_EXCLUSIVE_LONG, ACTIONS(KEY_NONE), ACTIONS(KEY_LEFT_BRACKET)),
        Button_(0, HOLD_EXCLUSIVE_LONG, ACTIONS(KEY_NONE), ACTIONS(KEY_RIGHT_BRACKET)),
        Button_(0, NORMAL, ACTIONS(KEY_NONE)),
        Button_(0, NORMAL, ACTIONS(KEY_NONE)),
        Button_(PIN_L3, HOLD_EXCLUSIVE_LONG, ACTIONS(KEY_NONE), ACTIONS(PROC_THANKS)),
        Button_(0, NORMAL, ACTIONS(KEY_NONE)),
        Button_(0, NORMAL, ACTIONS(KEY_NONE))
    );

    profile.dhat = Dhat_(
        Button_(0, NORMAL, ACTIONS(KEY_F8)),
        Button_(0, HOLD_EXCLUSIVE_LONG, ACTIONS(KEY_F4), ACTIONS(PROC_TUNE_OS)),
        Button_(0, NORMAL, ACTIONS(KEY_F7)),
        Button_(0, HOLD_EXCLUSIVE_LONG, ACTIONS(KEY_F1), ACTIONS(PROC_TUNE_DEADZONE)),
        Button_(0, NORMAL, ACTIONS(KEY_BACKQUOTE)),
        Button_(0, HOLD_EXCLUSIVE_LONG, ACTIONS(KEY_F3), ACTIONS(PROC_TUNE_VIBRATION)),
        Button_(0, NORMAL, ACTIONS(KEY_F5)),
        Button_(0, HOLD_EXCLUSIVE_LONG, ACTIONS(KEY_F2), ACTIONS(PROC_TUNE_SENSITIVITY)),
        Button_(0, NORMAL, ACTIONS(KEY_F6))
    );

    profile.gyro = Gyro_(
        GYRO_MODE_TOUCH_ON,
        ACTIONS(MOUSE_X),
        ACTIONS(MOUSE_Y),
        ACTIONS(KEY_NONE)
    );

    return profile;
}
