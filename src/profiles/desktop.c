// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include "button.h"
#include "hid.h"
#include "pin.h"
#include "thumbstick.h"
#include "glyph.h"
#include "gyro.h"
#include "config.h"

void config_profile_default_desktop(CtrlProfile *profile){
    // Metadata.
    profile->sections[SECTION_META].meta = (CtrlProfileMeta){
        .name = "Desktop",
        .control_byte = NVM_CONTROL_BYTE,
        .version_major = NVM_PROFILE_VERSION / 1000000,
        .version_minor = (NVM_PROFILE_VERSION / 1000) % 1000,
        .version_patch = NVM_PROFILE_VERSION % 1000,
    };

    // ABXY.
    profile->sections[SECTION_A].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_ENTER},
    };
    profile->sections[SECTION_B].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_ESCAPE},
    };
    profile->sections[SECTION_X].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_BACKSPACE},
    };
    profile->sections[SECTION_Y].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_SPACE},
    };

    // DPad.
    profile->sections[SECTION_DPAD_LEFT].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_LEFT},
    };
    profile->sections[SECTION_DPAD_RIGHT].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_RIGHT},
    };
    profile->sections[SECTION_DPAD_UP].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_UP},
    };
    profile->sections[SECTION_DPAD_DOWN].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_DOWN},
    };

    // Select/Start.
    profile->sections[SECTION_SELECT_1].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_CONTROL_LEFT, KEY_Z},
        .hint="Undo",
    };
    profile->sections[SECTION_START_1].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_CONTROL_LEFT, KEY_SHIFT_LEFT, KEY_Z},
        .hint="Redo",
    };
    profile->sections[SECTION_SELECT_2].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_CONTROL_LEFT, KEY_C},
        .hint="Copy",
    };
    profile->sections[SECTION_START_2].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_CONTROL_LEFT, KEY_V},
        .hint="Paste",
    };

    // Triggers.
    profile->sections[SECTION_L1].button = (CtrlButton){
        .mode=NORMAL,
        .actions={MOUSE_3},
    };
    profile->sections[SECTION_R1].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_ALT_LEFT},
    };
    profile->sections[SECTION_L2].button = (CtrlButton){
        .mode=NORMAL,
        .actions={MOUSE_2},
    };
    profile->sections[SECTION_R2].button = (CtrlButton){
        .mode=NORMAL,
        .actions={MOUSE_1},
    };
    profile->sections[SECTION_L4].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_CONTROL_LEFT},
    };
    profile->sections[SECTION_R4].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_SHIFT_LEFT},
    };

    // Thumbstick (left).
    profile->sections[SECTION_LSTICK_SETTINGS].thumbstick = (CtrlThumbstick){
        .mode=THUMBSTICK_MODE_ALPHANUMERIC,
        .saturation=100,
        .overlap=50,
    };

    // Thumbstick (right) / DHat.
    profile->sections[SECTION_RSTICK_SETTINGS].thumbstick = (CtrlThumbstick){
        .mode=THUMBSTICK_MODE_8DIR,
        .saturation=70,
        .overlap=50,
        .deadzone_override=true,
        .deadzone=60,
    };
    profile->sections[SECTION_RSTICK_UL].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_1},
    };
    profile->sections[SECTION_RSTICK_UP].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_2},
    };
    profile->sections[SECTION_RSTICK_UR].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_3},
    };
    profile->sections[SECTION_RSTICK_LEFT].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_4},
    };
    profile->sections[SECTION_RSTICK_PUSH].button = (CtrlButton){
        .mode=HOLD,
        .actions={KEY_5},
        .actions_secondary={KEY_0},
    };
    profile->sections[SECTION_RSTICK_RIGHT].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_6},
    };
    profile->sections[SECTION_RSTICK_DL].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_7},
    };
    profile->sections[SECTION_RSTICK_DOWN].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_8},
    };
    profile->sections[SECTION_RSTICK_DR].button = (CtrlButton){
        .mode=NORMAL,
        .actions={KEY_9},
    };

    // Rotary.
    profile->sections[SECTION_ROTARY_UP].rotary = (CtrlRotary){
        .actions_0={MOUSE_SCROLL_UP},
    };
    profile->sections[SECTION_ROTARY_DOWN].rotary = (CtrlRotary){
        .actions_0={MOUSE_SCROLL_DOWN},
    };

    // Glyph-stick.
    CtrlGlyph glyphs[44] = {
        {.actions={KEY_A}, .glyph=glyph_encode((Glyph){DIR4_LEFT})},
        {.actions={KEY_E}, .glyph=glyph_encode((Glyph){DIR4_RIGHT})},
        {.actions={KEY_I}, .glyph=glyph_encode((Glyph){DIR4_DOWN})},
        {.actions={KEY_O}, .glyph=glyph_encode((Glyph){DIR4_UP})},
        {.actions={KEY_U}, .glyph=glyph_encode((Glyph){DIR4_LEFT, DIR4_DOWN, DIR4_RIGHT})},
        {.actions={KEY_A}, .glyph=glyph_encode((Glyph){DIR4_LEFT, DIR4_DOWN, DIR4_RIGHT, DIR8_UP})},
        {.actions={KEY_B}, .glyph=glyph_encode((Glyph){DIR4_DOWN, DIR4_RIGHT, DIR4_UP})},
        {.actions={KEY_C}, .glyph=glyph_encode((Glyph){DIR4_UP, DIR4_LEFT, DIR4_DOWN})},
        {.actions={KEY_D}, .glyph=glyph_encode((Glyph){DIR4_UP, DIR4_RIGHT, DIR4_DOWN})},
        {.actions={KEY_E}, .glyph=glyph_encode((Glyph){DIR4_RIGHT, DIR4_UP, DIR4_LEFT, DIR4_DOWN})},
        {.actions={KEY_F}, .glyph=glyph_encode((Glyph){DIR4_UP, DIR4_RIGHT, DIR4_DOWN, DIR4_LEFT})},
        {.actions={KEY_G}, .glyph=glyph_encode((Glyph){DIR4_DOWN, DIR4_LEFT, DIR4_UP})},
        {.actions={KEY_H}, .glyph=glyph_encode((Glyph){DIR4_DOWN, DIR4_RIGHT, DIR4_DOWN})},
        {.actions={KEY_J}, .glyph=glyph_encode((Glyph){DIR4_DOWN, DIR4_LEFT})},
        {.actions={KEY_K}, .glyph=glyph_encode((Glyph){DIR4_UP, DIR4_RIGHT, DIR4_UP})},
        {.actions={KEY_L}, .glyph=glyph_encode((Glyph){DIR4_DOWN, DIR4_RIGHT})},
        {.actions={KEY_M}, .glyph=glyph_encode((Glyph){DIR4_LEFT, DIR4_UP, DIR4_RIGHT})},
        {.actions={KEY_N}, .glyph=glyph_encode((Glyph){DIR4_UP, DIR4_RIGHT})},
        {.actions={KEY_O}, .glyph=glyph_encode((Glyph){DIR4_UP, DIR4_LEFT, DIR4_DOWN, DIR4_RIGHT, DIR4_UP})},
        {.actions={KEY_O}, .glyph=glyph_encode((Glyph){DIR4_UP, DIR4_RIGHT, DIR4_DOWN, DIR4_LEFT, DIR4_UP})},
        {.actions={KEY_P}, .glyph=glyph_encode((Glyph){DIR4_RIGHT, DIR4_UP, DIR4_LEFT})},
        {.actions={KEY_Q}, .glyph=glyph_encode((Glyph){DIR4_UP, DIR4_LEFT, DIR4_DOWN, DIR4_RIGHT})},
        {.actions={KEY_R}, .glyph=glyph_encode((Glyph){DIR4_RIGHT, DIR4_UP})},
        {.actions={KEY_S}, .glyph=glyph_encode((Glyph){DIR4_RIGHT, DIR4_DOWN})},
        {.actions={KEY_T}, .glyph=glyph_encode((Glyph){DIR4_UP, DIR4_LEFT})},
        {.actions={KEY_V}, .glyph=glyph_encode((Glyph){DIR4_LEFT, DIR4_DOWN})},
        {.actions={KEY_W}, .glyph=glyph_encode((Glyph){DIR4_LEFT, DIR4_DOWN, DIR4_LEFT})},
        {.actions={KEY_X}, .glyph=glyph_encode((Glyph){DIR4_RIGHT, DIR4_DOWN, DIR4_RIGHT})},
        {.actions={KEY_Y}, .glyph=glyph_encode((Glyph){DIR4_RIGHT, DIR4_DOWN, DIR4_LEFT})},
        {.actions={KEY_Z}, .glyph=glyph_encode((Glyph){DIR4_RIGHT, DIR4_DOWN, DIR4_LEFT, DIR4_DOWN, DIR4_RIGHT})},
        {.actions={KEY_COMMA}, .glyph=glyph_encode((Glyph){DIR4_LEFT, DIR4_UP})},
        {.actions={KEY_PERIOD}, .glyph=glyph_encode((Glyph){DIR4_LEFT, DIR4_UP, DIR4_LEFT})},
        {.actions={KEY_SHIFT_LEFT, KEY_2}, .glyph=glyph_encode((Glyph){DIR4_DOWN, DIR4_RIGHT, DIR4_UP, DIR4_LEFT, DIR4_DOWN})},  // @
        {.actions={KEY_SHIFT_LEFT, KEY_SLASH}, .glyph=glyph_encode((Glyph){DIR4_DOWN, DIR4_RIGHT, DIR4_UP, DIR4_LEFT})},  // ?
        // More glyphs can be added here.
    };
    for(uint8_t s=0; s<4; s++) {
        uint8_t section = SECTION_GLYPHS_0 + s;
        profile->sections[section].glyphs = (CtrlGlyphs){.glyphs={}};
        for(uint8_t i=0; i<11; i++) {
            uint8_t glyph_index = (s * 11) + i;
            profile->sections[section].glyphs.glyphs[i] = glyphs[glyph_index];
        }
    }

    // Daisywheel.
    CtrlDaisyGroup up =         {.actions_a={KEY_A}, .actions_b={KEY_B}, .actions_x={KEY_C}, .actions_y={KEY_D}};
    CtrlDaisyGroup up_right =   {.actions_a={KEY_E}, .actions_b={KEY_F}, .actions_x={KEY_G}, .actions_y={KEY_H}};
    CtrlDaisyGroup left =       {.actions_a={KEY_I}, .actions_b={KEY_J}, .actions_x={KEY_K}, .actions_y={KEY_L}};
    CtrlDaisyGroup right =      {.actions_a={KEY_O}, .actions_b={KEY_M}, .actions_x={KEY_N}, .actions_y={KEY_NONE}};
    CtrlDaisyGroup down_left =  {.actions_a={KEY_P}, .actions_b={KEY_Q}, .actions_x={KEY_R}, .actions_y={KEY_S}};
    CtrlDaisyGroup down =       {.actions_a={KEY_U}, .actions_b={KEY_T}, .actions_x={KEY_V}, .actions_y={KEY_NONE}};
    CtrlDaisyGroup down_right = {.actions_a={KEY_W}, .actions_b={KEY_Z}, .actions_x={KEY_X}, .actions_y={KEY_Y}};
    CtrlDaisyGroup up_left =    {
        .actions_a={KEY_COMMA},
        .actions_b={KEY_PERIOD},
        .actions_x={KEY_SHIFT_LEFT, KEY_2},      // @
        .actions_y={KEY_SHIFT_LEFT, KEY_SLASH},  // ?
    };
    profile->sections[SECTION_DAISY_0].daisy = (CtrlDaisy){.groups={left, right}};
    profile->sections[SECTION_DAISY_1].daisy = (CtrlDaisy){.groups={up, down}};
    profile->sections[SECTION_DAISY_2].daisy = (CtrlDaisy){.groups={up_left, up_right}};
    profile->sections[SECTION_DAISY_3].daisy = (CtrlDaisy){.groups={down_left, down_right}};

    // Gyro.
    profile->sections[SECTION_GYRO_SETTINGS].gyro = (CtrlGyro){
        .mode=GYRO_MODE_TOUCH_ON,
        .engage=PIN_TOUCH_IN,
    };
    profile->sections[SECTION_GYRO_X].gyro_axis = (CtrlGyroAxis){
        .actions_neg={MOUSE_X_NEG},
        .actions_pos={MOUSE_X},
        .hint_pos="Mouse",
    };
    profile->sections[SECTION_GYRO_Y].gyro_axis = (CtrlGyroAxis){
        .actions_neg={MOUSE_Y_NEG},
        .actions_pos={MOUSE_Y},
        .hint_pos="Mouse",
    };
}
