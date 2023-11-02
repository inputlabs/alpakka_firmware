// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include <string.h>
#include "hid.h"
#include "pin.h"
#include "button.h"
#include "webusb.h"

void config_profile_default_fps_fusion(CtrlProfile profile){
    CtrlProfile profile_default = {
        {0},
        /* NAME       */  "FPS Fusion",

        /* A          */  {NORMAL, KEY_F},
        /* B          */  {NORMAL, KEY_V},
        /* X          */  {NORMAL, KEY_R},
        /* Y          */  {NORMAL, KEY_T},

        /* DPAD_LEFT  */  {NORMAL, KEY_LEFT},
        /* DPAD_RIGHT */  {NORMAL, KEY_RIGHT},
        /* DPAD_UP    */  {NORMAL, KEY_UP},
        /* DPAD_DOWN  */  {NORMAL, KEY_DOWN},

        /* SELECT_1   */  {NORMAL, KEY_TAB},
        /* START_1    */  {NORMAL, KEY_ESCAPE},
        /* SELECT_2   */  {NORMAL, KEY_M},
        /* START_2    */  {NORMAL, KEY_N},

        /* L1         */  {NORMAL, KEY_Q},
        /* R1         */  {NORMAL, KEY_E},

        /* L2         */  {NORMAL, MOUSE_2},
        /* R2         */  {NORMAL, MOUSE_1},

        /* L4         */  {NORMAL, KEY_SPACE},
        /* R4         */  {HOLD_OVERLAP, KEY_LEFT_CONTROL, 0, 0, 0, KEY_P},

        /* DHAT_LEFT  */  {NORMAL, KEY_1},
        /* DHAT_RIGHT */  {NORMAL, KEY_3},
        /* DHAT_UP    */  {NORMAL, KEY_4},
        /* DHAT_DOWN  */  {NORMAL, KEY_2},
        /* DHAT_UL    */  {NORMAL, KEY_8},
        /* DHAT_UR    */  {NORMAL, KEY_7},
        /* DHAT_DL    */  {NORMAL, KEY_5},
        /* DHAT_DR    */  {NORMAL, KEY_6},
        /* DHAT_PUSH  */  {NORMAL, KEY_0},
    };
    memcpy(profile, profile_default, 64*64);
}
