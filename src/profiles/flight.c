// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include "hid.h"
#include "pin.h"
#include "webusb.h"
#include "button.h"
#include "thumbstick.h"

void config_profile_default_flight(CtrlProfile *profile){
    // Profile name.
    profile->sections[SECTION_NAME].name = (CtrlProfileName){.name="Flight"};

    // ABXY.
    profile->sections[SECTION_A].button = (CtrlButton){};
    profile->sections[SECTION_B].button = (CtrlButton){};
    profile->sections[SECTION_X].button = (CtrlButton){};
    profile->sections[SECTION_Y].button = (CtrlButton){};

    // DPad.
    profile->sections[SECTION_DPAD_LEFT].button = (CtrlButton){};
    profile->sections[SECTION_DPAD_RIGHT].button = (CtrlButton){};
    profile->sections[SECTION_DPAD_UP].button = (CtrlButton){};
    profile->sections[SECTION_DPAD_DOWN].button = (CtrlButton){};

    // Select/Start.
    profile->sections[SECTION_SELECT_1].button = (CtrlButton){};
    profile->sections[SECTION_START_1].button = (CtrlButton){};
    profile->sections[SECTION_SELECT_2].button = (CtrlButton){};
    profile->sections[SECTION_START_2].button = (CtrlButton){};

    // Triggers.
    profile->sections[SECTION_L1].button = (CtrlButton){};
    profile->sections[SECTION_R1].button = (CtrlButton){};
    profile->sections[SECTION_L2].button = (CtrlButton){};
    profile->sections[SECTION_R2].button = (CtrlButton){};
    profile->sections[SECTION_L4].button = (CtrlButton){};
    profile->sections[SECTION_R4].button = (CtrlButton){};

    // DHat.
    profile->sections[SECTION_DHAT_LEFT].button = (CtrlButton){};
    profile->sections[SECTION_DHAT_RIGHT].button = (CtrlButton){};
    profile->sections[SECTION_DHAT_UP].button = (CtrlButton){};
    profile->sections[SECTION_DHAT_DOWN].button = (CtrlButton){};
    profile->sections[SECTION_DHAT_UL].button = (CtrlButton){};
    profile->sections[SECTION_DHAT_UR].button = (CtrlButton){};
    profile->sections[SECTION_DHAT_DL].button = (CtrlButton){};
    profile->sections[SECTION_DHAT_DR].button = (CtrlButton){};
    profile->sections[SECTION_DHAT_PUSH].button = (CtrlButton){};

    // Rotary.
    profile->sections[SECTION_ROTARY_UP].rotary = (CtrlRotary){};
    profile->sections[SECTION_ROTARY_DOWN].rotary = (CtrlRotary){};

    // Thumbstick.
    profile->sections[SECTION_THUMBSTICK].thumbstick = (CtrlThumbstick){
        .mode=THUMBSTICK_MODE_OFF,
    };
}
