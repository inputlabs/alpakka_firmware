// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include "pin.h"
#include "hid.h"
#include "button.h"
#include "profile.h"

Profile profile_init_console_legacy() {
    Profile profile = Profile_();

    profile.select_1 =   Button_( PIN_SELECT_1,   NORMAL, ACTIONS(GAMEPAD_SELECT) );
    profile.select_2 =   Button_( PIN_SELECT_2,   NORMAL, ACTIONS(KEY_NONE) );
    profile.start_1 =    Button_( PIN_START_1,    NORMAL, ACTIONS(GAMEPAD_START) );
    profile.start_2 =    Button_( PIN_START_2,    NORMAL, ACTIONS(KEY_NONE) );

    profile.dpad_left =  Button_( PIN_DPAD_LEFT,  NORMAL, ACTIONS(GAMEPAD_LEFT) );
    profile.dpad_right = Button_( PIN_DPAD_RIGHT, NORMAL, ACTIONS(GAMEPAD_RIGHT) );
    profile.dpad_up =    Button_( PIN_DPAD_UP,    NORMAL, ACTIONS(GAMEPAD_UP) );
    profile.dpad_down =  Button_( PIN_DPAD_DOWN,  NORMAL, ACTIONS(GAMEPAD_DOWN) );

    profile.a =          Button_( PIN_A,          NORMAL, ACTIONS(GAMEPAD_A) );
    profile.b =          Button_( PIN_B,          NORMAL, ACTIONS(GAMEPAD_B) );
    profile.x =          Button_( PIN_X,          NORMAL, ACTIONS(GAMEPAD_X) );
    profile.y =          Button_( PIN_Y,          NORMAL, ACTIONS(GAMEPAD_Y) );

    profile.l1 =         Button_( PIN_L1,         NORMAL, ACTIONS(GAMEPAD_L1) );
    profile.r1 =         Button_( PIN_R1,         NORMAL, ACTIONS(GAMEPAD_R1) );
    profile.l2 =         Button_( PIN_L2,         NORMAL, ACTIONS(GAMEPAD_AXIS_LZ) );
    profile.r2 =         Button_( PIN_R2,         NORMAL, ACTIONS(GAMEPAD_AXIS_RZ) );
    profile.l4 =         Button_( PIN_L4,         NORMAL, ACTIONS(GAMEPAD_A) );
    profile.r4 =         Button_( PIN_R4,         NORMAL, ACTIONS(GAMEPAD_B) );

    profile.rotary =     Rotary_( 0, ACTIONS(MOUSE_SCROLL_UP), ACTIONS(MOUSE_SCROLL_DOWN));

    profile.thumbstick = Thumbstick_(
        Button_(0, NORMAL, ACTIONS(GAMEPAD_AXIS_LX)),
        Button_(0, NORMAL, ACTIONS(GAMEPAD_AXIS_LX)),
        Button_(0, NORMAL, ACTIONS(GAMEPAD_AXIS_LY)),
        Button_(0, NORMAL, ACTIONS(GAMEPAD_AXIS_LY)),
        Button_(PIN_L3, NORMAL, ACTIONS(GAMEPAD_L3)),
        Button_(0, NORMAL, ACTIONS(KEY_NONE)),
        Button_(0, NORMAL, ACTIONS(KEY_NONE))
    );

    profile.dhat = Dhat_(
        Button_(0, NORMAL, ACTIONS(GAMEPAD_AXIS_RX_NEG, GAMEPAD_AXIS_RY_NEG)),
        Button_(0, NORMAL, ACTIONS(GAMEPAD_AXIS_RY_NEG)),
        Button_(0, NORMAL, ACTIONS(GAMEPAD_AXIS_RX, GAMEPAD_AXIS_RY_NEG)),

        Button_(0, NORMAL, ACTIONS(GAMEPAD_AXIS_RX_NEG)),
        Button_(0, NORMAL, ACTIONS(GAMEPAD_R3)),
        Button_(0, NORMAL, ACTIONS(GAMEPAD_AXIS_RX)),

        Button_(0, NORMAL, ACTIONS(GAMEPAD_AXIS_RX_NEG, GAMEPAD_AXIS_RY)),
        Button_(0, NORMAL, ACTIONS(GAMEPAD_AXIS_RY)),
        Button_(0, NORMAL, ACTIONS(GAMEPAD_AXIS_RX, GAMEPAD_AXIS_RY))
    );

    profile.gyro = Gyro_(
        GYRO_MODE_ALWAYS_OFF,
        ACTIONS(KEY_NONE),
        ACTIONS(KEY_NONE),
        ACTIONS(KEY_NONE)
    );

    return profile;
}
