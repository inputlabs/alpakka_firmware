// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include "pin.h"
#include "hid.h"
#include "button.h"
#include "profile.h"

Profile profile_init_desktop() {
    Profile profile = Profile_();

    profile.select_1 = Button_(PIN_SELECT_1, NORMAL, ACTIONS(KEY_LEFT_CONTROL, KEY_Z));
    profile.select_2 = Button_(PIN_SELECT_2, NORMAL, ACTIONS(KEY_LEFT_CONTROL, KEY_C));
    profile.start_1 =  Button_(PIN_START_1,  NORMAL, ACTIONS(KEY_LEFT_CONTROL, KEY_LEFT_SHIFT, KEY_Z));
    profile.start_2 =  Button_(PIN_START_2,  NORMAL, ACTIONS(KEY_LEFT_CONTROL, KEY_V));

    profile.dpad_left =  Button_(PIN_DPAD_LEFT,  NORMAL, ACTIONS(KEY_LEFT));
    profile.dpad_right = Button_(PIN_DPAD_RIGHT, NORMAL, ACTIONS(KEY_RIGHT));
    profile.dpad_up =    Button_(PIN_DPAD_UP,    NORMAL, ACTIONS(KEY_UP));
    profile.dpad_down =  Button_(PIN_DPAD_DOWN,  NORMAL, ACTIONS(KEY_DOWN));

    profile.a = Button_(PIN_A, NORMAL, ACTIONS(KEY_ENTER));
    profile.b = Button_(PIN_B, NORMAL, ACTIONS(KEY_ESCAPE));
    profile.x = Button_(PIN_X, NORMAL, ACTIONS(KEY_BACKSPACE));
    profile.y = Button_(PIN_Y, NORMAL, ACTIONS(KEY_SPACE));

    profile.l1 = Button_(PIN_L1, NORMAL, ACTIONS(KEY_NONE));
    profile.r1 = Button_(PIN_R1, NORMAL, ACTIONS(KEY_LEFT_ALT));
    profile.l2 = Button_(PIN_L2, NORMAL, ACTIONS(MOUSE_2));
    profile.r2 = Button_(PIN_R2, NORMAL, ACTIONS(MOUSE_1));
    profile.l4 = Button_(PIN_L4, NORMAL, ACTIONS(KEY_LEFT_SHIFT));
    profile.r4 = Button_(PIN_R4, NORMAL, ACTIONS(KEY_LEFT_CONTROL));

    profile.rotary = Rotary_(NULL, ACTIONS(MOUSE_SCROLL_UP), ACTIONS(MOUSE_SCROLL_DOWN));

    profile.thumbstick = Thumbstick_(
        THUMBSTICK_MODE_GLYPH,  // Mode.
        DEADZONE_FROM_CONFIG,   // Deadzone.
        0.5,                    // Overlap.
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_NONE)),  // Left.
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_NONE)),  // Right.
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_NONE)),  // Up.
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_NONE)),  // Down.
        Button_(PIN_L3,      NORMAL, ACTIONS(KEY_NONE)),  // Push.
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_NONE)),  // Inner.
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_NONE))   // Outer.
    );

    profile.thumbstick.config_daisywheel(
        &profile.thumbstick,
        ACTIONS(KEY_I), ACTIONS(KEY_J), ACTIONS(KEY_K), ACTIONS(KEY_L),      // Left.
        ACTIONS(KEY_O), ACTIONS(KEY_M), ACTIONS(KEY_N), ACTIONS(KEY_NONE),   // Right.
        ACTIONS(KEY_A), ACTIONS(KEY_B), ACTIONS(KEY_C), ACTIONS(KEY_D),      // Up.
        ACTIONS(KEY_U), ACTIONS(KEY_T), ACTIONS(KEY_V), ACTIONS(KEY_NONE),   // Down.
        ACTIONS(KEY_COMMA), ACTIONS(KEY_PERIOD),                             // ↖
        ACTIONS(KEY_LEFT_SHIFT, KEY_2), ACTIONS(KEY_LEFT_SHIFT, KEY_SLASH),
        ACTIONS(KEY_E), ACTIONS(KEY_F), ACTIONS(KEY_G), ACTIONS(KEY_H),      // ↗
        ACTIONS(KEY_P), ACTIONS(KEY_Q), ACTIONS(KEY_R), ACTIONS(KEY_S),      // ↙
        ACTIONS(KEY_W), ACTIONS(KEY_Z), ACTIONS(KEY_X), ACTIONS(KEY_Y)       // ↘
    );

    profile.dhat = Dhat_(
        // Emulating a numeric keypad.
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_4)),  // Left.
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_6)),  // Right.
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_2)),  // Up.
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_8)),  // Down.
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_1)),  // ↖
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_3)),  // ↗
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_7)),  // ↙
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_9)),  // ↘
        Button_(PIN_VIRTUAL, HOLD_EXCLUSIVE, ACTIONS(KEY_5), ACTIONS(KEY_0))  // Push.
    );

    profile.gyro = Gyro_(
        GYRO_MODE_TOUCH_ON,
        PIN_TOUCH_IN,
        ACTIONS(MOUSE_X),
        ACTIONS(MOUSE_Y),
        ACTIONS(KEY_NONE)
    );

    return profile;
}
