// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include "pin.h"
#include "hid.h"
#include "button.h"
#include "profile.h"
#include "webusb.h"


CtrlProfile ctrl_init_fps_fusion() {
    CtrlProfile profile = {
        (Ctrl){0, 0, 0, 0, 0, 'F', 'P', 'S', ' ', 'F', 'u', 's', 'i', 'o', 'n'},

        (Ctrl){0, 0, 0, 0, 0, SECTION_A,          NORMAL,       KEY_F},
        (Ctrl){0, 0, 0, 0, 0, SECTION_B,          NORMAL,       KEY_V},
        (Ctrl){0, 0, 0, 0, 0, SECTION_X,          NORMAL,       KEY_R},
        (Ctrl){0, 0, 0, 0, 0, SECTION_Y,          NORMAL,       KEY_T},

        (Ctrl){0, 0, 0, 0, 0, SECTION_DPAD_LEFT,  NORMAL,       KEY_LEFT},
        (Ctrl){0, 0, 0, 0, 0, SECTION_DPAD_RIGHT, NORMAL,       KEY_RIGHT},
        (Ctrl){0, 0, 0, 0, 0, SECTION_DPAD_UP,    NORMAL,       KEY_UP},
        (Ctrl){0, 0, 0, 0, 0, SECTION_DPAD_DOWN,  NORMAL,       KEY_DOWN},

        (Ctrl){0, 0, 0, 0, 0, SECTION_SELECT_1,   NORMAL,       KEY_TAB},
        (Ctrl){0, 0, 0, 0, 0, SECTION_START_1,    NORMAL,       KEY_ESCAPE},
        (Ctrl){0, 0, 0, 0, 0, SECTION_SELECT_2,   NORMAL,       KEY_M},
        (Ctrl){0, 0, 0, 0, 0, SECTION_START_2,    NORMAL,       KEY_N},

        (Ctrl){0, 0, 0, 0, 0, SECTION_L1,         NORMAL,       KEY_Q},
        (Ctrl){0, 0, 0, 0, 0, SECTION_R1,         NORMAL,       KEY_E},
        (Ctrl){0, 0, 0, 0, 0, SECTION_L2,         NORMAL,       MOUSE_2},
        (Ctrl){0, 0, 0, 0, 0, SECTION_R2,         NORMAL,       MOUSE_1},
        (Ctrl){0, 0, 0, 0, 0, SECTION_L4,         NORMAL,       KEY_SPACE},
        (Ctrl){0, 0, 0, 0, 0, SECTION_R4,         HOLD_OVERLAP, KEY_LEFT_CONTROL, 0, 0, 0, KEY_P},

        (Ctrl){0, 0, 0, 0, 0, SECTION_DHAT_LEFT,  NORMAL,       KEY_1},
        (Ctrl){0, 0, 0, 0, 0, SECTION_DHAT_RIGHT, NORMAL,       KEY_3},
        (Ctrl){0, 0, 0, 0, 0, SECTION_DHAT_UP,    NORMAL,       KEY_4},
        (Ctrl){0, 0, 0, 0, 0, SECTION_DHAT_DOWN,  NORMAL,       KEY_2},
        (Ctrl){0, 0, 0, 0, 0, SECTION_DHAT_UL,    NORMAL,       KEY_8},
        (Ctrl){0, 0, 0, 0, 0, SECTION_DHAT_UR,    NORMAL,       KEY_7},
        (Ctrl){0, 0, 0, 0, 0, SECTION_DHAT_DL,    NORMAL,       KEY_5},
        (Ctrl){0, 0, 0, 0, 0, SECTION_DHAT_DR,    NORMAL,       KEY_6},
        (Ctrl){0, 0, 0, 0, 0, SECTION_DHAT_PUSH,  NORMAL,       KEY_0},
    };
    return profile;
;
}


// Profile profile_init_fps_fusion() {
//     Profile profile = Profile_();

//     profile.select_1 = Button_(PIN_SELECT_1, NORMAL, ACTIONS(KEY_TAB));
//     profile.start_1 =  Button_(PIN_START_1,  NORMAL, ACTIONS(KEY_ESCAPE));
//     profile.select_2 = Button_(PIN_SELECT_2, NORMAL, ACTIONS(KEY_M));
//     profile.start_2 =  Button_(PIN_START_2,  NORMAL, ACTIONS(KEY_N));

//     profile.dpad_left =  Button_(PIN_DPAD_LEFT,  NORMAL, ACTIONS(KEY_LEFT));
//     profile.dpad_right = Button_(PIN_DPAD_RIGHT, NORMAL, ACTIONS(KEY_RIGHT));
//     profile.dpad_up =    Button_(PIN_DPAD_UP,    NORMAL, ACTIONS(KEY_UP));
//     profile.dpad_down =  Button_(PIN_DPAD_DOWN,  NORMAL, ACTIONS(KEY_DOWN));

//     profile.a = Button_(PIN_A, NORMAL, ACTIONS(KEY_F));
//     profile.b = Button_(PIN_B, NORMAL, ACTIONS(KEY_V));
//     profile.x = Button_(PIN_X, NORMAL, ACTIONS(KEY_R));
//     profile.y = Button_(PIN_Y, NORMAL, ACTIONS(KEY_T));

//     profile.l1 = Button_(PIN_L1, NORMAL, ACTIONS(KEY_Q));
//     profile.r1 = Button_(PIN_R1, NORMAL, ACTIONS(KEY_E));
//     profile.l2 = Button_(PIN_L2, NORMAL, ACTIONS(MOUSE_2));
//     profile.r2 = Button_(PIN_R2, NORMAL, ACTIONS(MOUSE_1));
//     profile.l4 = Button_(PIN_L4, NORMAL, ACTIONS(KEY_SPACE));
//     profile.r4 = Button_(PIN_R4, HOLD_OVERLAP, ACTIONS(KEY_LEFT_CONTROL), ACTIONS(KEY_P));

//     profile.rotary = Rotary_(NULL, ACTIONS(MOUSE_SCROLL_UP), ACTIONS(MOUSE_SCROLL_DOWN));

//     profile.thumbstick = Thumbstick_(
//         THUMBSTICK_MODE_4DIR,                                        // Mode.
//         DEADZONE_FROM_CONFIG,                                        // Deadzone.
//         0.5,                                                         // Overlap.
//         Button_(PIN_VIRTUAL, NORMAL, ACTIONS(GAMEPAD_AXIS_LX_NEG)),  // Left.
//         Button_(PIN_VIRTUAL, NORMAL, ACTIONS(GAMEPAD_AXIS_LX)),      // Right.
//         Button_(PIN_VIRTUAL, NORMAL, ACTIONS(GAMEPAD_AXIS_LY_NEG)),  // Up.
//         Button_(PIN_VIRTUAL, NORMAL, ACTIONS(GAMEPAD_AXIS_LY)),      // Down.
//         Button_(PIN_L3,      NORMAL, ACTIONS(KEY_LEFT_SHIFT)),       // Push.
//         Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_LEFT_BRACKET)),     // Inner.
//         Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_RIGHT_BRACKET))     // Outer.
//     );

//     profile.dhat = Dhat_(
//         Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_1)),  // Left.
//         Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_3)),  // Right.
//         Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_4)),  // Up.
//         Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_2)),  // Down.
//         Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_8)),  // ↖
//         Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_7)),  // ↗
//         Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_5)),  // ↙
//         Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_6)),  // ↘
//         Button_(PIN_VIRTUAL, NORMAL, ACTIONS(KEY_0))   // Push.
//     );

//     profile.gyro = Gyro_(
//         GYRO_MODE_TOUCH_ON,
//         PIN_TOUCH_IN,
//         ACTIONS(MOUSE_X_NEG), ACTIONS(MOUSE_X),  // X rotation.
//         ACTIONS(MOUSE_Y_NEG), ACTIONS(MOUSE_Y),  // Y rotation.
//         ACTIONS(KEY_NONE), ACTIONS(KEY_NONE)     // Z rotation.
//     );

//     return profile;
// }
