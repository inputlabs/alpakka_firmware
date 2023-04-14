// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <pico/stdlib.h>
#include <hardware/adc.h>
#include "config.h"
#include "pin.h"
#include "button.h"
#include "thumbstick.h"
#include "helper.h"
#include "hid.h"
#include "led.h"
#include "profile.h" // Experimental.

float offset_x = 0;
float offset_y = 0;
float config_deadzone = 0;

// TODO: Experimental.
float CUT4 = 45;
float CUT4_ = 135;  // 180 - 45
float CUT8 = 22.5;
bool daisy_used = false;
Button daisy_a;
Button daisy_b;
Button daisy_x;
Button daisy_y;

float thumbstick_adc(uint8_t adc_index, float offset) {
    adc_select_input(adc_index);
    float value = (float)adc_read() - 2048;
    value = value / 2048 * CFG_THUMBSTICK_SATURATION;
    return limit_between(value - offset, -1, 1);
}

void thumbstick_update_deadzone() {
    config_nvm_t config;
    config_read(&config);
    float deadzones[3] = {
        CFG_THUMBSTICK_DEADZONE_LOW,
        CFG_THUMBSTICK_DEADZONE_MID,
        CFG_THUMBSTICK_DEADZONE_HIGH
    };
    config_deadzone = deadzones[config.deadzone];
}

void thumbstick_update_offsets() {
    config_nvm_t config;
    config_read(&config);
    offset_x = config.ts_offset_x;
    offset_y = config.ts_offset_y;
}

void thumbstick_calibrate() {
    printf("Thumbstick: calibrating...\n");
    float x = 0;
    float y = 0;
    uint32_t len = 100000;
    for(uint32_t i=0; i<len; i++) {
        if (!(i % 5000)) led_cycle_step();
        x += thumbstick_adc(1, 0.0);
        y += thumbstick_adc(0, 0.0);
    }
    x /= len;
    y /= len;
    printf("Thumbstick: calibration x=%f y=%f\n", x, y);
    config_set_thumbstick_offset(x, y);
    thumbstick_update_offsets();
}

void thumbstick_init() {
    printf("INIT: Thumbstick\n");
    adc_init();
    adc_gpio_init(PIN_TX);
    adc_gpio_init(PIN_TY);
    thumbstick_update_offsets();
    thumbstick_update_deadzone();
    // TODO: Experimental.
    daisy_a = Button_(PIN_A,  NORMAL, ACTIONS(KEY_NONE));
    daisy_b = Button_(PIN_B,  NORMAL, ACTIONS(KEY_NONE));
    daisy_x = Button_(PIN_X,  NORMAL, ACTIONS(KEY_NONE));
    daisy_y = Button_(PIN_Y,  NORMAL, ACTIONS(KEY_NONE));
}

void thumbstick_report_axis(uint8_t axis, float value) {
    if      (axis == GAMEPAD_AXIS_LX)     hid_gamepad_lx( value * ANALOG_FACTOR);
    else if (axis == GAMEPAD_AXIS_LY)     hid_gamepad_ly(-value * ANALOG_FACTOR);
    else if (axis == GAMEPAD_AXIS_RX)     hid_gamepad_rx( value * ANALOG_FACTOR);
    else if (axis == GAMEPAD_AXIS_RY)     hid_gamepad_ry(-value * ANALOG_FACTOR);
    else if (axis == GAMEPAD_AXIS_LX_NEG) hid_gamepad_lx(-value * ANALOG_FACTOR);
    else if (axis == GAMEPAD_AXIS_LY_NEG) hid_gamepad_ly( value * ANALOG_FACTOR);
    else if (axis == GAMEPAD_AXIS_RX_NEG) hid_gamepad_rx(-value * ANALOG_FACTOR);
    else if (axis == GAMEPAD_AXIS_RY_NEG) hid_gamepad_ry( value * ANALOG_FACTOR);
    else if (axis == GAMEPAD_AXIS_LZ) hid_gamepad_lz(max(0, value) * TRIGGER_FACTOR);
    else if (axis == GAMEPAD_AXIS_RZ) hid_gamepad_rz(max(0, value) * TRIGGER_FACTOR);
}

void Thumbstick__report_4dir(
    Thumbstick *self,
    ThumbstickPosition pos,
    float deadzone
) {
    // Evaluate virtual buttons.
    if (pos.radius > deadzone) {
        if (pos.radius < CFG_THUMBSTICK_INNER_RADIUS) self->inner.virtual_press = true;
        else self->outer.virtual_press = true;
        float cutA = 45 * (-self->overlap + 1);
        float cutB = 180 - cutA;
        if (is_between(pos.angle, -cutB, -cutA)) self->left.virtual_press = true;
        if (is_between(pos.angle, cutA, cutB)) self->right.virtual_press = true;
        if (fabs(pos.angle) <= 90 - cutA) self->up.virtual_press = true;
        if (fabs(pos.angle) >= 90 + cutA) self->down.virtual_press = true;
    }
    // Report directional virtual buttons or axis.
    if (!hid_is_axis(self->left.actions[0])) self->left.report(&self->left);
    else if (pos.x < 0) thumbstick_report_axis(self->left.actions[0], -pos.x);
    if (!hid_is_axis(self->right.actions[0])) self->right.report(&self->right);
    else if (pos.x >= 0) thumbstick_report_axis(self->right.actions[0], pos.x);
    if (!hid_is_axis(self->up.actions[0])) self->up.report(&self->up);
    else if (pos.y < 0) thumbstick_report_axis(self->up.actions[0], -pos.y);
    if (!hid_is_axis(self->down.actions[0])) self->down.report(&self->down);
    else if (pos.y >= 0) thumbstick_report_axis(self->down.actions[0], pos.y);
    // Report inner and outer (only if calibrated).
    if (offset_x != 0 && offset_y != 0) {
        self->inner.report(&self->inner);
        self->outer.report(&self->outer);
    }
    // Report push.
    self->push.report(&self->push);
}

// TODO: Experimental.
uint8_t glyphs[32][8] = {
    {ACTIONS(KEY_A, DIR4_LEFT, DIR4_DOWN, DIR4_RIGHT, DIR8_UP)},
    {ACTIONS(KEY_B, DIR4_DOWN, DIR4_RIGHT, DIR4_UP)},
    {ACTIONS(KEY_C, DIR4_UP, DIR4_LEFT, DIR4_DOWN)},
    {ACTIONS(KEY_D, DIR4_UP, DIR4_RIGHT, DIR4_DOWN)},
    {ACTIONS(KEY_E, DIR4_RIGHT, DIR4_UP, DIR4_LEFT, DIR4_DOWN)},
    {ACTIONS(KEY_F, DIR4_UP, DIR4_RIGHT, DIR4_DOWN, DIR4_LEFT)},
    {ACTIONS(KEY_G, DIR4_DOWN, DIR4_LEFT, DIR4_UP)},
    {ACTIONS(KEY_H, DIR4_DOWN, DIR4_RIGHT, DIR4_DOWN)},
    {ACTIONS(KEY_I, DIR4_DOWN)},
    {ACTIONS(KEY_J, DIR4_DOWN, DIR4_LEFT)},
    {ACTIONS(KEY_K, DIR4_UP, DIR4_RIGHT, DIR4_UP)},
    {ACTIONS(KEY_L, DIR4_DOWN, DIR4_RIGHT)},
    {ACTIONS(KEY_M, DIR4_LEFT, DIR4_UP, DIR4_RIGHT)},
    {ACTIONS(KEY_N, DIR4_UP, DIR4_RIGHT)},
    {ACTIONS(KEY_O, DIR4_UP, DIR4_LEFT, DIR4_DOWN, DIR4_RIGHT, DIR4_UP)},
    {ACTIONS(KEY_P, DIR4_RIGHT, DIR4_UP, DIR4_LEFT)},
    {ACTIONS(KEY_Q, DIR4_UP, DIR4_LEFT, DIR4_DOWN, DIR4_RIGHT)},
    {ACTIONS(KEY_R, DIR4_RIGHT, DIR4_UP)},
    {ACTIONS(KEY_S, DIR4_RIGHT, DIR4_DOWN)},
    {ACTIONS(KEY_T, DIR4_UP, DIR4_LEFT)},
    {ACTIONS(KEY_U, DIR4_LEFT, DIR4_DOWN, DIR4_RIGHT)},
    {ACTIONS(KEY_V, DIR4_LEFT, DIR4_DOWN)},
    {ACTIONS(KEY_W, DIR4_LEFT, DIR4_DOWN, DIR4_LEFT)},
    {ACTIONS(KEY_X, DIR4_RIGHT, DIR4_DOWN, DIR4_RIGHT)},
    {ACTIONS(KEY_Y, DIR4_RIGHT, DIR4_DOWN, DIR4_LEFT)},
    {ACTIONS(KEY_Z, DIR4_RIGHT, DIR4_DOWN, DIR4_LEFT, DIR4_DOWN, DIR4_RIGHT)},
    {ACTIONS(KEY_COMMA, DIR4_LEFT, DIR4_UP)},
    {ACTIONS(KEY_PERIOD, DIR4_LEFT, DIR4_UP, DIR4_LEFT)},
    // Shortcuts and alts.
    {ACTIONS(KEY_A, DIR4_LEFT)},
    {ACTIONS(KEY_E, DIR4_RIGHT)},
    {ACTIONS(KEY_O, DIR4_UP, DIR4_RIGHT, DIR4_DOWN, DIR4_LEFT, DIR4_UP)},
    {ACTIONS(KEY_O, DIR4_UP)},
};

// TODO: Experimental.
bool thumbstick_glyph_match(Dir4 *input, uint8_t *glyph) {
    for(uint8_t i=0; i<8; i++) {
        if (input[i] == glyph[i+1]) {
            continue;
        } else {
            if (input[i] == 0 && glyph[i+1] == SENTINEL) {
                hid_press(glyph[0]);
                hid_release_later(glyph[0], 100);
                return true;
            } else {
                break;
            }
        }
    }
    return false;
}

// TODO: Experimental.
void set_daisy(uint8_t key_a, uint8_t key_b, uint8_t key_x, uint8_t key_y) {
    if      (daisy_a.is_pressed(&daisy_a)) {hid_press(key_a); hid_release_later(key_a, 10); daisy_used=true;}
    else if (daisy_b.is_pressed(&daisy_b)) {hid_press(key_b); hid_release_later(key_b, 10); daisy_used=true;}
    else if (daisy_x.is_pressed(&daisy_x)) {hid_press(key_x); hid_release_later(key_x, 10); daisy_used=true;}
    else if (daisy_y.is_pressed(&daisy_y)) {hid_press(key_y); hid_release_later(key_y, 10); daisy_used=true;}
}

// TODO: Experimental.
void Thumbstick__report_glyph(Thumbstick *self, ThumbstickPosition pos) {
    // Glyph-stick + daisy implementation.
    static Dir4 input[8] = {0,};
    static uint8_t input_index = 0;
    Dir4 dir4 = 0;
    Dir8 dir8 = 0;
    if (pos.radius > 0.7) {
        lock_abxy = true;
        // Detect direction 4.
        if      (is_between(pos.angle, -CUT4_, -CUT4)) dir4 = DIR4_LEFT;
        else if (is_between(pos.angle,  CUT4,  CUT4_)) dir4 = DIR4_RIGHT;
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
        else if (fabs(pos.angle) >= CUT8*7)              dir8 = DIR4_DOWN;
        // Record direction 4.
        if (input_index == 0 || dir4 != input[input_index-1]) {
            input[input_index] = dir4;
            input_index += 1;
        }
        // Report daisy keyboard.
        if      (dir8 == DIR8_UP)         set_daisy(KEY_A, KEY_B, KEY_C, KEY_D);
        else if (dir8 == DIR8_UP_RIGHT)   set_daisy(KEY_E, KEY_H, KEY_G, KEY_F);
        else if (dir8 == DIR8_RIGHT)      set_daisy(KEY_O, KEY_N, KEY_M, KEY_NONE);
        else if (dir8 == DIR8_DOWN_RIGHT) set_daisy(KEY_W, KEY_Z, KEY_X, KEY_Y);
        else if (dir8 == DIR8_DOWN)       set_daisy(KEY_U, KEY_T, KEY_V, KEY_NONE);
        else if (dir8 == DIR8_DOWN_LEFT)  set_daisy(KEY_S, KEY_R, KEY_P, KEY_Q);
        else if (dir8 == DIR8_LEFT)       set_daisy(KEY_I, KEY_L, KEY_J, KEY_K);
        else if (dir8 == DIR8_UP_LEFT)    set_daisy(KEY_COMMA, KEY_PERIOD, KEY_NONE, KEY_NONE);
    } else {
        if (input_index > 0) {
            // Glyph-stick match.
            if (!daisy_used) {
                for(uint8_t i=0; i<32; i++) {
                    if (thumbstick_glyph_match(input, glyphs[i])) break;
                }
            }
            // Glyph-stick reset.
            memset(input, 0, sizeof(input));
            input_index = 0;
            // Daisywheel reset.
            daisy_used = false;
            lock_abxy = false;
        }
    }
}

void Thumbstick__report(Thumbstick *self) {
    // Get values from ADC.
    float x = thumbstick_adc(1, offset_x);
    float y = thumbstick_adc(0, offset_y);
    // Calculate trigonometry.
    float angle = atan2(x, -y) * (180 / M_PI);
    float radius = sqrt(powf(x, 2) + powf(y, 2));
    float deadzone = self->deadzone == DEADZONE_FROM_CONFIG ? config_deadzone : self->deadzone;
    radius = limit_between(radius, 0, 1);
    radius = ramp_low(radius, deadzone);
    x = sin(radians(angle)) * radius;
    y = -cos(radians(angle)) * radius;
    ThumbstickPosition pos = {x, y, angle, radius};
    // Report.
    if (self->mode == THUMBSTICK_MODE_4DIR) self->report_4dir(self, pos, deadzone);
    else if (self->mode == THUMBSTICK_MODE_GLYPH) self->report_glyph(self, pos);
}

void Thumbstick__reset(Thumbstick *self) {
    self->left.reset(&self->left);
    self->right.reset(&self->right);
    self->up.reset(&self->up);
    self->down.reset(&self->down);
    self->push.reset(&self->push);
    self->inner.reset(&self->inner);
    self->outer.reset(&self->inner);
}

Thumbstick Thumbstick_ (
    ThumbstickMode mode,
    float deadzone,
    float overlap,
    Button left,
    Button right,
    Button up,
    Button down,
    Button push,
    Button inner,
    Button outer
) {
    Thumbstick thumbstick;
    thumbstick.mode = mode;
    thumbstick.report = Thumbstick__report;
    thumbstick.report_4dir = Thumbstick__report_4dir;
    thumbstick.report_glyph = Thumbstick__report_glyph;
    thumbstick.reset = Thumbstick__reset;
    thumbstick.deadzone = deadzone;
    thumbstick.overlap = overlap;
    thumbstick.left = left;
    thumbstick.right = right;
    thumbstick.up = up;
    thumbstick.down = down;
    thumbstick.inner = inner;
    thumbstick.outer = outer;
    thumbstick.push = push;
    return thumbstick;
}
