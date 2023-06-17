// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include "pin.h"
#include "hid.h"
#include "button.h"
#include "profile.h"

Profile profile_init_racing() {
    Profile profile = Profile_();

    profile.select_1 = Button_(PIN_SELECT_1, NORMAL, ACTIONS(GAMEPAD_SELECT));
    profile.select_2 = Button_(PIN_SELECT_2, NORMAL, ACTIONS(KEY_NONE));
    profile.start_1 =  Button_(PIN_START_1,  NORMAL, ACTIONS(GAMEPAD_START));
    profile.start_2 =  Button_(PIN_START_2,  NORMAL, ACTIONS(KEY_NONE));

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
    profile.l2 = Button_(PIN_L2, NORMAL, ACTIONS(GAMEPAD_L3));
    profile.r2 = Button_(PIN_R2, NORMAL, ACTIONS(GAMEPAD_R3));
    profile.l4 = Button_(PIN_L4, NORMAL, ACTIONS(GAMEPAD_LEFT));
    profile.r4 = Button_(PIN_R4, NORMAL, ACTIONS(GAMEPAD_RIGHT));

    profile.rotary = Rotary_(NULL, ACTIONS(KEY_PAD_1), ACTIONS(KEY_PAD_2));
    profile.rotary.config_mode(&profile.rotary, 1, ACTIONS(KEY_PAD_3), ACTIONS(KEY_PAD_4));
    profile.rotary.config_mode(&profile.rotary, 2, ACTIONS(KEY_PAD_5), ACTIONS(KEY_PAD_6));
    profile.rotary.config_mode(&profile.rotary, 3, ACTIONS(KEY_PAD_7), ACTIONS(KEY_PAD_8));
    profile.rotary.config_mode(&profile.rotary, 4, ACTIONS(PROC_ADZ), ACTIONS(PROC_ADZN));

    profile.thumbstick = Thumbstick_(
        THUMBSTICK_MODE_RADIAL,                                  // Mode.
        DEADZONE_FROM_CONFIG,                                    // Deadzone.
        0.8,                                                     // Overlap.
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_NONE)),         // Left.
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_NONE)),         // Right.
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(GAMEPAD_AXIS_RZ)),  // Up.
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(GAMEPAD_AXIS_LZ)),  // Down.
        Button_(PIN_L3,      NORMAL, ACTIONS(GAMEPAD_UP)),       // Push.
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_NONE)),         // Inner.
        Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_NONE))          // Outer.
    );

    profile.dhat = Dhat_(
        // Left.
        Button_(
            PIN_VIRTUAL,
            HOLD_OVERLAP,
            ACTIONS(GAMEPAD_AXIS_RX_NEG, PROC_ROTARY_MODE_0),
            ACTIONS(PROC_ROTARY_MODE_4)
        ),
        // Right.
        Button_(PIN_VIRTUAL,
            NORMAL,
            ACTIONS(GAMEPAD_AXIS_RX, PROC_ROTARY_MODE_2)
        ),
        // Up.
        Button_(PIN_VIRTUAL,
            NORMAL,
            ACTIONS(GAMEPAD_AXIS_RY_NEG, PROC_ROTARY_MODE_3)
        ),
        // Down.
        Button_(PIN_VIRTUAL,
            NORMAL,
            ACTIONS(GAMEPAD_AXIS_RY, PROC_ROTARY_MODE_1)
        ),
        // ↖
        Button_(PIN_VIRTUAL,
            NORMAL,
            ACTIONS(GAMEPAD_AXIS_RX_NEG, GAMEPAD_AXIS_RY_NEG)
        ),
        // ↗
        Button_(PIN_VIRTUAL,
            NORMAL,
            ACTIONS(GAMEPAD_AXIS_RX, GAMEPAD_AXIS_RY_NEG)
        ),
        // ↙
        Button_(PIN_VIRTUAL,
            NORMAL,
            ACTIONS(GAMEPAD_AXIS_RX_NEG, GAMEPAD_AXIS_RY)
        ),
        // ↘
        Button_(PIN_VIRTUAL,
            NORMAL,
            ACTIONS(GAMEPAD_AXIS_RX, GAMEPAD_AXIS_RY)
        ),
        // Push.
        Button_(PIN_VIRTUAL,
            NORMAL,
            ACTIONS(GAMEPAD_DOWN)
        )
    );

    profile.gyro = Gyro_(
        GYRO_MODE_AXIS_ABSOLUTE,
        PIN_NONE,
        ACTIONS(GAMEPAD_AXIS_LX),
        ACTIONS(KEY_NONE),
        ACTIONS(KEY_NONE)
    );

    return profile;
}
