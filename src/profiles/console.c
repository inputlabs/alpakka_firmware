// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include "pin.h"
#include "hid.h"
#include "button.h"
#include "profile.h"

Profile profile_init_console() {
    Profile profile = Profile_();

    profile.select_1 = Button_(PIN_SELECT_1, NORMAL, ACTIONS(GAMEPAD_SELECT));
    profile.select_2 = Button_(PIN_SELECT_2, NORMAL, ACTIONS(KEY_M));
    profile.start_1 =  Button_(PIN_START_1,  NORMAL, ACTIONS(GAMEPAD_START));
    profile.start_2 =  Button_(PIN_START_2,  NORMAL, ACTIONS(KEY_N));

    profile.dpad_left =  Button_(PIN_DPAD_LEFT,  NORMAL, ACTIONS(GAMEPAD_LEFT));
    profile.dpad_right = Button_(PIN_DPAD_RIGHT, NORMAL, ACTIONS(GAMEPAD_RIGHT));
    profile.dpad_up =    Button_(PIN_DPAD_UP,    NORMAL, ACTIONS(GAMEPAD_UP));
    profile.dpad_down =  Button_(PIN_DPAD_DOWN,  NORMAL, ACTIONS(GAMEPAD_DOWN));

    profile.a = Button_(PIN_A, NORMAL, ACTIONS(GAMEPAD_A));
    profile.b = Button_(PIN_B, NORMAL, ACTIONS(GAMEPAD_B));
    profile.x = Button_(PIN_X, NORMAL, ACTIONS(GAMEPAD_X));
    profile.y = Button_(PIN_Y, NORMAL, ACTIONS(GAMEPAD_Y));

    profile.l1 = Button_(PIN_L1, NORMAL, ACTIONS(GAMEPAD_L1));
    profile.r1 = Button_(PIN_R1, NORMAL, ACTIONS(GAMEPAD_R1));
    profile.l2 = Button_(PIN_L2, NORMAL, ACTIONS(GAMEPAD_AXIS_LZ));
    profile.r2 = Button_(PIN_R2, NORMAL, ACTIONS(GAMEPAD_AXIS_RZ));
    profile.l4 = Button_(PIN_L4, NORMAL, ACTIONS(KEY_SPACE));
    profile.r4 = Button_(PIN_R4, NORMAL, ACTIONS(KEY_LEFT_CONTROL));

    profile.rotary = Rotary_(NULL, ACTIONS(MOUSE_SCROLL_UP), ACTIONS(MOUSE_SCROLL_DOWN));

    profile.thumbstick = Thumbstick_(
        THUMBSTICK_MODE_4DIR,                                        // Mode.
        DEADZONE_FROM_CONFIG,                                        // Deadzone.
        0.5,                                                         // Overlap.
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(GAMEPAD_AXIS_LX_NEG)),  // Left.
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(GAMEPAD_AXIS_LX)),      // Right.
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(GAMEPAD_AXIS_LY_NEG)),  // Up.
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(GAMEPAD_AXIS_LY)),      // Down.
        Button_(PIN_L3,      NORMAL, ACTIONS(GAMEPAD_L3)),           // Push.
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_NONE)),             // Inner.
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_NONE))              // Outer.
    );

    profile.dhat = Dhat_(
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_1)),      // Left.
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_3)),      // Right.
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_4)),      // Up.
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_2)),      // Down.
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_8)),      // ↖
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_7)),      // ↗
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_5)),      // ↙
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_6)),      // ↘
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(GAMEPAD_R3))  // Push.
    );

    profile.gyro = Gyro_(
        GYRO_MODE_TOUCH_ON,
        PIN_TOUCH_IN,
        ACTIONS(MOUSE_X_NEG), ACTIONS(MOUSE_X),  // X rotation.
        ACTIONS(MOUSE_Y_NEG), ACTIONS(MOUSE_Y),  // Y rotation.
        ACTIONS(KEY_NONE), ACTIONS(KEY_NONE)     // Z rotation.
    );

    printf("OK\n");
    return profile;
}
