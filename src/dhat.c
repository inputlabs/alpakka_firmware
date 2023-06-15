// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include <stdlib.h>
#include <pico/time.h>
#include "config.h"
#include "pin.h"
#include "button.h"
#include "dhat.h"
#include "hid.h"
#include "led.h"

void Dhat__update(Dhat *self) {
    // Evaluate real buttons.
    bool left = self->left.is_pressed(&self->left);
    bool right = self->right.is_pressed(&self->right);
    bool up = self->up.is_pressed(&self->up);
    bool down = self->down.is_pressed(&self->down);
    bool push = self->push.is_pressed(&self->push);
    // Debounce.
    if (left || right || up || down || push) {
        if (time_us_64() <= self->timestamp + CFG_DHAT_DEBOUNCE_TIME*1000) {
            return;
        }
        self->timestamp = time_us_64();
    }
    // Report on virtual buttons.
    self->up_left.virtual_press = (up && left);
    self->up_center.virtual_press = (up && !left && !right);
    self->up_right.virtual_press = (up && right);
    self->mid_left.virtual_press = (left && !up && !down);
    self->mid_right.virtual_press = (right && !up && !down);
    self->down_left.virtual_press = (down && left);
    self->down_right.virtual_press = (down && right);
    self->down_center.virtual_press = (down && !left && !right);
    self->mid_center.virtual_press = (push && !left && !right && !up && !down);
}

void Dhat__report(Dhat *self) {
    self->update(self);
    self->up_left.report(&self->up_left);
    self->up_center.report(&self->up_center);
    self->up_right.report(&self->up_right);
    self->mid_left.report(&self->mid_left);
    self->mid_center.report(&self->mid_center);
    self->mid_right.report(&self->mid_right);
    self->down_left.report(&self->down_left);
    self->down_center.report(&self->down_center);
    self->down_right.report(&self->down_right);
}

void Dhat__reset(Dhat *self) {
    self->left.reset(&self->left);
    self->right.reset(&self->right);
    self->up.reset(&self->up);
    self->down.reset(&self->down);
    self->push.reset(&self->push);
    self->up_left.reset(&self->up_left);
    self->up_center.reset(&self->up_center);
    self->up_right.reset(&self->up_right);
    self->mid_left.reset(&self->mid_left);
    self->mid_center.reset(&self->mid_center);
    self->mid_right.reset(&self->mid_right);
    self->down_left.reset(&self->down_left);
    self->down_center.reset(&self->down_center);
    self->down_right.reset(&self->down_right);
}

Dhat Dhat_ (
    Button mid_left,
    Button mid_right,
    Button up_center,
    Button down_center,
    Button up_left,
    Button up_right,
    Button down_left,
    Button down_right,
    Button mid_center
) {
    Dhat dhat;
    dhat.update = Dhat__update;
    dhat.report = Dhat__report;
    dhat.reset = Dhat__reset;
    dhat.timestamp = 0;
    // Real buttons.
    dhat.left =  Button_(PIN_DHAT_LEFT,  NORMAL, ACTIONS(KEY_NONE));
    dhat.right = Button_(PIN_DHAT_RIGHT, NORMAL, ACTIONS(KEY_NONE));
    dhat.up =    Button_(PIN_DHAT_UP,    NORMAL, ACTIONS(KEY_NONE));
    dhat.down =  Button_(PIN_DHAT_DOWN,  NORMAL, ACTIONS(KEY_NONE));
    dhat.push =  Button_(PIN_DHAT_PUSH,  NORMAL, ACTIONS(KEY_NONE));
    // Virtual buttons.
    dhat.up_left = up_left;
    dhat.up_center = up_center;
    dhat.up_right = up_right;
    dhat.mid_left = mid_left;
    dhat.mid_center = mid_center;
    dhat.mid_right = mid_right;
    dhat.down_left = down_left;
    dhat.down_center = down_center;
    dhat.down_right = down_right;
    return dhat;
}
