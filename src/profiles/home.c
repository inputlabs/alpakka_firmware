// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
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
        ACTIONS(KEY_CONTROL_LEFT, KEY_ALT_LEFT, KEY_DELETE)
    );
    profile.start_2 = Button_(
        PIN_START_2,
        HOLD_EXCLUSIVE_LONG,
        ACTIONS(KEY_F12),
        ACTIONS(PROC_CALIBRATE)
    );

    profile.dpad_left =  Button_(PIN_DPAD_LEFT,  HOLD_OVERLAP, ACTIONS(PROC_PROFILE_4), ACTIONS(PROC_PROFILE_8));
    profile.dpad_right = Button_(PIN_DPAD_RIGHT, HOLD_OVERLAP, ACTIONS(PROC_PROFILE_2), ACTIONS(PROC_PROFILE_6));
    profile.dpad_up =    Button_(PIN_DPAD_UP,    HOLD_OVERLAP, ACTIONS(PROC_PROFILE_1), ACTIONS(PROC_PROFILE_5));
    profile.dpad_down =  Button_(PIN_DPAD_DOWN,  HOLD_OVERLAP, ACTIONS(PROC_PROFILE_3), ACTIONS(PROC_PROFILE_7));

    profile.a = Button_(PIN_A, NORMAL, ACTIONS(KEY_ENTER));
    profile.b = Button_(PIN_B, NORMAL, ACTIONS(KEY_ESCAPE));
    profile.x = Button_(PIN_X, NORMAL, ACTIONS(KEY_BACKSPACE));
    profile.y = Button_(PIN_Y, NORMAL, ACTIONS(KEY_SPACE));

    profile.l1 = Button_(
        PIN_L1,
        HOLD_EXCLUSIVE_LONG,
        ACTIONS(KEY_SUPER_LEFT, KEY_D),
        ACTIONS(KEY_ALT_LEFT, KEY_ENTER)
    );
    profile.r1 = Button_(PIN_R1, STICKY, ACTIONS(KEY_ALT_LEFT), ACTIONS(KEY_TAB));
    profile.l2 = Button_(PIN_L2, NORMAL, ACTIONS(MOUSE_2));
    profile.r2 = Button_(PIN_R2, NORMAL, ACTIONS(MOUSE_1));
    profile.l4 = Button_(PIN_L4, NORMAL, ACTIONS(KEY_NONE));
    profile.r4 = Button_(PIN_R4, NORMAL, ACTIONS(KEY_NONE));

    profile.rotary = Rotary_(NULL, ACTIONS(PROC_TUNE_UP), ACTIONS(PROC_TUNE_DOWN));

    profile.thumbstick = Thumbstick_(
        THUMBSTICK_MODE_4DIR,  // Mode.
        0.25,                  // Deadzone.
        -0.5,                  // Overlap.
        // Left.
        Button_(PIN_VIRTUAL, HOLD_EXCLUSIVE,
            ACTIONS(PROC_MACRO, KEY_S, KEY_O, KEY_R, KEY_R, KEY_Y, KEY_SPACE),
            ACTIONS(PROC_MACRO, KEY_N, KEY_O, KEY_SPACE)
        ),
        // Right.
        Button_(PIN_VIRTUAL, HOLD_EXCLUSIVE,
            ACTIONS(PROC_MACRO, KEY_O, KEY_K, KEY_SPACE),
            ACTIONS(PROC_MACRO, KEY_Y, KEY_E, KEY_S, KEY_SPACE)
        ),
        // Up.
        Button_(PIN_VIRTUAL, HOLD_EXCLUSIVE,
            ACTIONS(PROC_MACRO, KEY_H, KEY_E, KEY_L, KEY_L, KEY_O, KEY_SPACE),
            ACTIONS(PROC_MACRO, KEY_G, KEY_G, KEY_SPACE)
        ),
        // Down.
        Button_(PIN_VIRTUAL, HOLD_EXCLUSIVE,
            ACTIONS(PROC_MACRO, KEY_N, KEY_I, KEY_C, KEY_E, KEY_SPACE),
            ACTIONS(PROC_MACRO, KEY_T, KEY_H, KEY_A, KEY_N, KEY_K, KEY_S, KEY_SPACE)
        ),
        // Push.
        Button_(
            PIN_L3,
            HOLD_EXCLUSIVE_LONG,
            ACTIONS(KEY_SUPER_LEFT, KEY_CONTROL_LEFT, KEY_O),
            ACTIONS(PROC_THANKS)
        ),
        // Inner.
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_NONE)),
        // Outer.
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_NONE))
    );

    profile.dhat = Dhat_(
        Button_(PIN_VIRTUAL, HOLD_EXCLUSIVE_LONG, ACTIONS(KEY_F1), ACTIONS(PROC_TUNE_DEADZONE)),         // Left.
        Button_(PIN_VIRTUAL, HOLD_EXCLUSIVE_LONG, ACTIONS(KEY_F3), ACTIONS(PROC_TUNE_TOUCH_THRESHOLD)),  // Right.
        Button_(PIN_VIRTUAL, HOLD_EXCLUSIVE_LONG, ACTIONS(KEY_F4), ACTIONS(PROC_TUNE_OS)),               // Up.
        Button_(PIN_VIRTUAL, HOLD_EXCLUSIVE_LONG, ACTIONS(KEY_F2), ACTIONS(PROC_TUNE_SENSITIVITY)),      // Down.
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_F8)),                                                   // ↖
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_F7)),                                                   // ↗
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_F5)),                                                   // ↙
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_F6)),                                                   // ↘
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_BACKQUOTE))                                             // Push.
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
