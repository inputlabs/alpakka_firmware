// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include "pin.h"
#include "hid.h"
#include "button.h"
#include "profile.h"

Profile profile_init_fps_wasd() {
    Profile profile = Profile_();

    profile.select_1 = Button_(PIN_SELECT_1, NORMAL, ACTIONS(KEY_TAB));
    profile.start_1 =  Button_(PIN_START_1,  NORMAL, ACTIONS(KEY_ESCAPE));
    profile.select_2 = Button_(PIN_SELECT_2, NORMAL, ACTIONS(KEY_M));
    profile.start_2 =  Button_(PIN_START_2,  NORMAL, ACTIONS(KEY_N));

    profile.dpad_left =  Button_(PIN_DPAD_LEFT,  NORMAL, ACTIONS(KEY_LEFT));
    profile.dpad_right = Button_(PIN_DPAD_RIGHT, NORMAL, ACTIONS(KEY_RIGHT));
    profile.dpad_up =    Button_(PIN_DPAD_UP,    NORMAL, ACTIONS(KEY_UP));
    profile.dpad_down =  Button_(PIN_DPAD_DOWN,  NORMAL, ACTIONS(KEY_DOWN));

    profile.a = Button_(PIN_A, NORMAL, ACTIONS(KEY_F));
    profile.b = Button_(PIN_B, NORMAL, ACTIONS(KEY_V));
    profile.x = Button_(PIN_X, NORMAL, ACTIONS(KEY_R));
    profile.y = Button_(PIN_Y, NORMAL, ACTIONS(KEY_T));

    profile.l1 = Button_(PIN_L1, NORMAL, ACTIONS(KEY_Q));
    profile.r1 = Button_(PIN_R1, NORMAL, ACTIONS(KEY_E));
    profile.l2 = Button_(PIN_L2, NORMAL, ACTIONS(MOUSE_2));
    profile.r2 = Button_(PIN_R2, NORMAL, ACTIONS(MOUSE_1));
    profile.l4 = Button_(PIN_L4, NORMAL, ACTIONS(KEY_SPACE));
    profile.r4 = Button_(PIN_R4, HOLD_OVERLAP, ACTIONS(KEY_LEFT_CONTROL), ACTIONS(KEY_P));

    profile.rotary = Rotary_(NULL, ACTIONS(MOUSE_SCROLL_UP), ACTIONS(MOUSE_SCROLL_DOWN));

    profile.thumbstick = Thumbstick_(
        THUMBSTICK_MODE_4DIR,                                     // Mode.
        DEADZONE_FROM_CONFIG,                                     // Deadzone.
        0.5,                                                      // Overlap.
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_A)),             // Left.
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_D)),             // Right.
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_W)),             // Up.
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_S)),             // Down.
        Button_(PIN_L3,      NORMAL, ACTIONS(KEY_LEFT_SHIFT)),    // Push.
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_LEFT_BRACKET)),  // Inner.
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_RIGHT_BRACKET))  // Outer.
    );

    profile.dhat = Dhat_(
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_1)),  // Left.
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_3)),  // Right.
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_4)),  // Up.
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_2)),  // Down.
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_8)),  // ↖
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_7)),  // ↗
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_5)),  // ↙
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_6)),  // ↘
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_0))   // Push.
    );

    profile.gyro = Gyro_(
        GYRO_MODE_TOUCH_ON,
        PIN_TOUCH_IN,
        ACTIONS(MOUSE_X_NEG), ACTIONS(MOUSE_X),  // X rotation.
        ACTIONS(MOUSE_Y_NEG), ACTIONS(MOUSE_Y),  // Y rotation.
        ACTIONS(KEY_NONE), ACTIONS(KEY_NONE)     // Z rotation.
    );

    return profile;
}
