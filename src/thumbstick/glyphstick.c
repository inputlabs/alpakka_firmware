// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

/*
This file contains the logic for the thumbstick glyphstick and daisywheel modes.
*/

#include <stdio.h>
#include <string.h>
#include "thumbstick.h"
#include "config.h"
#include "button.h"
#include "pin.h"
#include "hid.h"
#include "profile.h"

// TODO: Refactor as inmemory struct instead of static.
bool daisywheel_used = false;
Button daisy_a;
Button daisy_b;
Button daisy_x;
Button daisy_y;

void thumbstick_init_daisywheel() {
    Actions none = {0,};
    daisy_a = Button_(PIN_A, NORMAL, none, none, none);
    daisy_b = Button_(PIN_B, NORMAL, none, none, none);
    daisy_x = Button_(PIN_X, NORMAL, none, none, none);
    daisy_y = Button_(PIN_Y, NORMAL, none, none, none);
}

void Thumbstick__config_glyphstick(Thumbstick *self, Actions actions, Glyph glyph) {
    uint8_t index = self->glyphstick_index;
    memcpy(self->glyphstick_actions[index], actions, 4);
    memcpy(self->glyphstick_glyphs[index], glyph, 5);
    self->glyphstick_index += 1;
}

void Thumbstick__report_glyphstick(Thumbstick *self, Glyph input) {
    // Iterate over all defined glyphs.
    uint8_t nglyphs = self->glyphstick_index;
    for(uint8_t i=0; i<nglyphs; i++) {
        // Pattern match user input against glyph.
        bool match = true;
        for(uint8_t j=0; j<5; j++) {
            if (input[j] != self->glyphstick_glyphs[i][j]) {
                match = false;
                break;
            }
        }
        // Trigger actions if matches.
        if (match) {
            hid_press_multiple(self->glyphstick_actions[i]);
            hid_release_multiple_later(self->glyphstick_actions[i], 100);
            break;
        }
    }
}

void Thumbstick__config_daisywheel(Thumbstick *self, uint8_t dir, uint8_t button, Actions actions) {
    memcpy(self->daisywheel[dir][button], actions, 4);
}

void Thumbstick__report_daisywheel(Thumbstick *self, Dir8 dir) {
    dir -= 1;  // Shift zero since not using center direction here.
    if (daisy_a.is_pressed(&daisy_a)) {
        hid_press_multiple(self->daisywheel[dir][0]);
        hid_release_multiple_later(self->daisywheel[dir][0], 10);
        daisywheel_used=true;
    }
    else if (daisy_b.is_pressed(&daisy_b)) {
        hid_press_multiple(self->daisywheel[dir][1]);
        hid_release_multiple_later(self->daisywheel[dir][1], 10);
        daisywheel_used=true;
    }
    else if (daisy_x.is_pressed(&daisy_x)) {
        hid_press_multiple(self->daisywheel[dir][2]);
        hid_release_multiple_later(self->daisywheel[dir][2], 10);
        daisywheel_used=true;
    }
    else if (daisy_y.is_pressed(&daisy_y)) {
        hid_press_multiple(self->daisywheel[dir][3]);
        hid_release_multiple_later(self->daisywheel[dir][3], 10);
        daisywheel_used=true;
    }
}

void Thumbstick__report_alphanumeric(Thumbstick *self, ThumbstickPosition pos) {
    static Glyph input = {0};
    static uint8_t input_index = 0;
    static float CUT4 = 45;
    static float CUT4X = 135;  // 180-45
    static float CUT8 = 22.5;
    Dir4 dir4 = 0;
    Dir8 dir8 = 0;
    if (pos.radius > 0.7) {
        profile_enable_abxy(false);
        // Detect direction 4.
        if      (is_between(pos.angle, -CUT4X, -CUT4)) dir4 = DIR4_LEFT;
        else if (is_between(pos.angle,  CUT4,  CUT4X)) dir4 = DIR4_RIGHT;
        else if (fabs(pos.angle) <= 90 - CUT4)         dir4 = DIR4_UP;
        else if (fabs(pos.angle) >= 90 + CUT4)         dir4 = DIR4_DOWN;
        // Detect direction 8.
        if      (is_between(pos.angle, -CUT8*1,  CUT8*1)) dir8 = DIR8_UP;
        else if (is_between(pos.angle,  CUT8*1,  CUT8*3)) dir8 = DIR8_UP_RIGHT;
        else if (is_between(pos.angle,  CUT8*3,  CUT8*5)) dir8 = DIR8_RIGHT;
        else if (is_between(pos.angle,  CUT8*5,  CUT8*7)) dir8 = DIR8_DOWN_RIGHT;
        else if (is_between(pos.angle, -CUT8*7, -CUT8*5)) dir8 = DIR8_DOWN_LEFT;
        else if (is_between(pos.angle, -CUT8*5, -CUT8*3)) dir8 = DIR8_LEFT;
        else if (is_between(pos.angle, -CUT8*3, -CUT8*1)) dir8 = DIR8_UP_LEFT;
        else if (fabs(pos.angle) >= CUT8*7)               dir8 = DIR8_DOWN;
        // Record direction 4.
        if (input_index < 5 && (input_index == 0 || dir4 != input[input_index-1])) {
            input[input_index] = dir4;
            input_index += 1;
        }
        // Report daisy keyboard.
        self->report_daisywheel(self, dir8);
    } else {
        if (input_index > 0) {
            // Glyph-stick match.
            if (!daisywheel_used) {
                self->report_glyphstick(self, input);
            }
            // Glyph-stick reset.
            memset(input, 0, 5);
            input_index = 0;
            // Daisywheel reset.
            daisywheel_used = false;
            profile_enable_abxy(true);
        }
    }
}
