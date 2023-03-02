// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include "pin.h"
#include "hid.h"
#include "button.h"
#include "profile.h"

Profile profile_init_desktop() {
    Profile profile = Profile_();

    profile.select_1 = Button_(
        PIN_SELECT_1,
        NORMAL,
        ACTIONS(KEY_LEFT_CONTROL, KEY_Z)
    );
    profile.select_2 = Button_(
        PIN_SELECT_2,
        NORMAL,
        ACTIONS(KEY_LEFT_CONTROL, KEY_C)
    );
    profile.start_1 = Button_(
        PIN_START_1,
        NORMAL,
        ACTIONS(KEY_LEFT_CONTROL, KEY_LEFT_SHIFT, KEY_Z)
    );
    profile.start_2 = Button_(
        PIN_START_2,
        NORMAL,
        ACTIONS(KEY_LEFT_CONTROL, KEY_V)
    );

    profile.dpad_left =  Button_( PIN_DPAD_LEFT,  NORMAL, ACTIONS(KEY_LEFT) );
    profile.dpad_right = Button_( PIN_DPAD_RIGHT, NORMAL, ACTIONS(KEY_RIGHT) );
    profile.dpad_up =    Button_( PIN_DPAD_UP,    NORMAL, ACTIONS(KEY_UP) );
    profile.dpad_down =  Button_( PIN_DPAD_DOWN,  NORMAL, ACTIONS(KEY_DOWN) );

    profile.a = Button_( PIN_A, NORMAL, ACTIONS(KEY_ENTER) );
    profile.b = Button_( PIN_B, NORMAL, ACTIONS(KEY_ESCAPE) );
    profile.x = Button_( PIN_X, NORMAL, ACTIONS(KEY_BACKSPACE) );
    profile.y = Button_( PIN_Y, NORMAL, ACTIONS(KEY_SPACE) );

    profile.l1 = Button_( PIN_L1, NORMAL, ACTIONS(MOUSE_3) );
    profile.r1 = Button_( PIN_R1, NORMAL, ACTIONS(KEY_LEFT_ALT) );
    profile.l2 = Button_( PIN_L2, NORMAL, ACTIONS(MOUSE_2) );
    profile.r2 = Button_( PIN_R2, NORMAL, ACTIONS(MOUSE_1) );
    profile.l4 = Button_( PIN_L4, NORMAL, ACTIONS(KEY_LEFT_SHIFT) );
    profile.r4 = Button_( PIN_R4, NORMAL, ACTIONS(KEY_LEFT_CONTROL) );

    profile.rotary = Rotary_( 0, ACTIONS(MOUSE_SCROLL_UP), ACTIONS(MOUSE_SCROLL_DOWN));

    profile.thumbstick = Thumbstick_(
        THUMBSTICK_MODE_KEYBOARD,
        // Left & right.
        Button_(PIN_NONE, CYCLE, ACTIONS(KEY_I, KEY_L, KEY_K, KEY_J)),
        Button_(PIN_NONE, CYCLE, ACTIONS(KEY_O, KEY_M, KEY_N)),
        // Up & Down
        Button_(PIN_NONE, CYCLE, ACTIONS(KEY_A, KEY_B, KEY_C, KEY_D)),
        Button_(PIN_NONE, CYCLE, ACTIONS(KEY_U, KEY_T, KEY_V)),
        // Corners.
        Button_(PIN_NONE, CYCLE, ACTIONS(KEY_COMMA, KEY_PERIOD)),
        Button_(PIN_NONE, CYCLE, ACTIONS(KEY_E, KEY_H, KEY_F, KEY_G)),
        Button_(PIN_NONE, CYCLE, ACTIONS(KEY_S, KEY_R, KEY_P, KEY_Q)),
        Button_(PIN_NONE, CYCLE, ACTIONS(KEY_W, KEY_Y, KEY_X, KEY_Z)),
        // Push, inner, outer.
        Button_(PIN_L3,   NORMAL, ACTIONS(KEY_NONE)),
        Button_(PIN_NONE, NORMAL, ACTIONS(KEY_NONE)),
        Button_(PIN_NONE, NORMAL, ACTIONS(KEY_NONE))
    );

    profile.dhat = Dhat_(
        Button_(PIN_NONE, NORMAL, ACTIONS(KEY_1)),
        Button_(PIN_NONE, NORMAL, ACTIONS(KEY_2)),
        Button_(PIN_NONE, NORMAL, ACTIONS(KEY_3)),
        Button_(PIN_NONE, NORMAL, ACTIONS(KEY_4)),
        Button_(PIN_NONE, HOLD_EXCLUSIVE, ACTIONS(KEY_5), ACTIONS(KEY_0)),
        Button_(PIN_NONE, NORMAL, ACTIONS(KEY_6)),
        Button_(PIN_NONE, NORMAL, ACTIONS(KEY_7)),
        Button_(PIN_NONE, NORMAL, ACTIONS(KEY_8)),
        Button_(PIN_NONE, NORMAL, ACTIONS(KEY_9))
    );

    profile.gyro = Gyro_(
        GYRO_MODE_TOUCH_ON,
        ACTIONS(MOUSE_X),
        ACTIONS(MOUSE_Y),
        ACTIONS(KEY_NONE)
    );

    return profile;
}
