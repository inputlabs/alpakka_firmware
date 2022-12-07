// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include "pin.h"
#include "hid.h"
#include "button.h"
#include "profile.h"

Profile profile_init_none() {
    Profile profile = Profile_();

    profile.select_1 = Button_( PIN_SELECT_1, NORMAL, ACTIONS(KEY_NONE) );
    profile.start_1 =  Button_( PIN_START_1,  NORMAL, ACTIONS(KEY_NONE) );
    profile.select_2 = Button_( PIN_SELECT_2, NORMAL, ACTIONS(KEY_NONE) );
    profile.start_2 =  Button_( PIN_START_2,  NORMAL, ACTIONS(KEY_NONE) );

    profile.dpad_left =  Button_( PIN_DPAD_LEFT,  NORMAL, ACTIONS(KEY_NONE) );
    profile.dpad_right = Button_( PIN_DPAD_RIGHT, NORMAL, ACTIONS(KEY_NONE) );
    profile.dpad_up =    Button_( PIN_DPAD_UP,    NORMAL, ACTIONS(KEY_NONE) );
    profile.dpad_down =  Button_( PIN_DPAD_DOWN,  NORMAL, ACTIONS(KEY_NONE) );

    profile.a = Button_( PIN_A, NORMAL, ACTIONS(KEY_NONE) );
    profile.b = Button_( PIN_B, NORMAL, ACTIONS(KEY_NONE) );
    profile.x = Button_( PIN_X, NORMAL, ACTIONS(KEY_NONE) );
    profile.y = Button_( PIN_Y, NORMAL, ACTIONS(KEY_NONE) );

    profile.l1 = Button_( PIN_L1, NORMAL, ACTIONS(KEY_NONE) );
    profile.r1 = Button_( PIN_R1, NORMAL, ACTIONS(KEY_NONE) );
    profile.l2 = Button_( PIN_L2, NORMAL, ACTIONS(KEY_NONE) );
    profile.r2 = Button_( PIN_R2, NORMAL, ACTIONS(KEY_NONE) );
    profile.l4 = Button_( PIN_L4, NORMAL, ACTIONS(KEY_NONE) );
    profile.r4 = Button_( PIN_R4, NORMAL, ACTIONS(KEY_NONE) );

    profile.rotary = Rotary_( 0, ACTIONS(KEY_NONE), ACTIONS(KEY_NONE));

    profile.thumbstick = Thumbstick_(
        Button_(0, NORMAL, ACTIONS(KEY_NONE)),
        Button_(0, NORMAL, ACTIONS(KEY_NONE)),
        Button_(0, NORMAL, ACTIONS(KEY_NONE)),
        Button_(0, NORMAL, ACTIONS(KEY_NONE)),
        Button_(PIN_L3, NORMAL, ACTIONS(KEY_NONE)),
        Button_(0, NORMAL, ACTIONS(KEY_NONE)),
        Button_(0, NORMAL, ACTIONS(KEY_NONE))
    );

    profile.dhat = Dhat_(
        Button_(0, NORMAL, ACTIONS(KEY_NONE)),
        Button_(0, NORMAL, ACTIONS(KEY_NONE)),
        Button_(0, NORMAL, ACTIONS(KEY_NONE)),
        Button_(0, NORMAL, ACTIONS(KEY_NONE)),
        Button_(0, NORMAL, ACTIONS(KEY_NONE)),
        Button_(0, NORMAL, ACTIONS(KEY_NONE)),
        Button_(0, NORMAL, ACTIONS(KEY_NONE)),
        Button_(0, NORMAL, ACTIONS(KEY_NONE)),
        Button_(0, NORMAL, ACTIONS(KEY_NONE))
    );

    profile.gyro = Gyro_(
        GYRO_MODE_ALWAYS_OFF,
        ACTIONS(KEY_NONE),
        ACTIONS(KEY_NONE),
        ACTIONS(KEY_NONE)
    );

    return profile;
}
