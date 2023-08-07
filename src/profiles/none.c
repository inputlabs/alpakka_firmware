// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include "pin.h"
#include "hid.h"
#include "button.h"
#include "profile.h"

Profile profile_init_none() {
    Profile profile = Profile_();
    printf("  None: ");

    profile.select_1 = Button_(PIN_SELECT_1, NORMAL, ACTIONS(KEY_NONE));
    profile.start_1 =  Button_(PIN_START_1,  NORMAL, ACTIONS(KEY_NONE));
    profile.select_2 = Button_(PIN_SELECT_2, NORMAL, ACTIONS(KEY_NONE));
    profile.start_2 =  Button_(PIN_START_2,  NORMAL, ACTIONS(KEY_NONE));

    profile.dpad_left =  Button_(PIN_DPAD_LEFT,  NORMAL, ACTIONS(KEY_NONE));
    profile.dpad_right = Button_(PIN_DPAD_RIGHT, NORMAL, ACTIONS(KEY_NONE));
    profile.dpad_up =    Button_(PIN_DPAD_UP,    NORMAL, ACTIONS(KEY_NONE));
    profile.dpad_down =  Button_(PIN_DPAD_DOWN,  NORMAL, ACTIONS(KEY_NONE));

    profile.a = Button_(PIN_A, NORMAL, ACTIONS(KEY_NONE));
    profile.b = Button_(PIN_B, NORMAL, ACTIONS(KEY_NONE));
    profile.x = Button_(PIN_X, NORMAL, ACTIONS(KEY_NONE));
    profile.y = Button_(PIN_Y, NORMAL, ACTIONS(KEY_NONE));

    profile.l1 = Button_(PIN_L1, NORMAL, ACTIONS(KEY_NONE));
    profile.r1 = Button_(PIN_R1, NORMAL, ACTIONS(KEY_NONE));
    profile.l2 = Button_(PIN_L2, NORMAL, ACTIONS(KEY_NONE));
    profile.r2 = Button_(PIN_R2, NORMAL, ACTIONS(KEY_NONE));
    profile.l4 = Button_(PIN_L4, NORMAL, ACTIONS(KEY_NONE));
    profile.r4 = Button_(PIN_R4, NORMAL, ACTIONS(KEY_NONE));

    profile.rotary = Rotary_(NULL, ACTIONS(KEY_NONE), ACTIONS(KEY_NONE));

    profile.thumbstick = Thumbstick_(
        THUMBSTICK_MODE_4DIR,                             // Mode.
        DEADZONE_FROM_CONFIG,                             // Deadzone.
        0.5,                                              // Overlap.
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_NONE)),  // Left.
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_NONE)),  // Right.
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_NONE)),  // Up.
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_NONE)),  // Down.
        Button_(PIN_L3,      NORMAL, ACTIONS(KEY_NONE)),  // Push.
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_NONE)),  // Inner.
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_NONE))   // Outer.
    );

    profile.dhat = Dhat_(
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_NONE)),  // Left.
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_NONE)),  // Right.
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_NONE)),  // Up.
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_NONE)),  // Down.
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_NONE)),  // ↖
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_NONE)),  // ↗
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_NONE)),  // ↙
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_NONE)),  // ↘
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_NONE))   // Push.
    );

    profile.gyro = Gyro_(
        GYRO_MODE_ALWAYS_OFF,
        PIN_NONE,
        ACTIONS(KEY_NONE), ACTIONS(KEY_NONE),  // X rotation.
        ACTIONS(KEY_NONE), ACTIONS(KEY_NONE),  // Y rotation.
        ACTIONS(KEY_NONE), ACTIONS(KEY_NONE)   // Z rotation.
    );

    printf("OK\n");
    return profile;
}
