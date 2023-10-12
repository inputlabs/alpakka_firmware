// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include <math.h>
#include <pico/stdlib.h>
#include <stdarg.h>
#include <hardware/adc.h>
#include "config.h"
#include "pin.h"
#include "button.h"
#include "thumbstick.h"
#include "helper.h"
#include "hid.h"
#include "led.h"
#include "profile.h"
#include "logging.h"

float offset_x = 0;
float offset_y = 0;
float config_deadzone = 0;

// Daisywheel.
bool daisywheel_used = false;
Button daisy_a;
Button daisy_b;
Button daisy_x;
Button daisy_y;

float thumbstick_adc(uint8_t adc_index, float offset) {
    adc_select_input(adc_index);
    float value = (float)adc_read() - BIT_11;
    value = value / BIT_11 * CFG_THUMBSTICK_SATURATION;
    return constrain(value - offset, -1, 1);
}

void thumbstick_update_deadzone() {
    uint8_t preset = config_get_deadzone_preset();
    config_deadzone = config_get_deadzone_value(preset);
}

void thumbstick_update_offsets() {
    config_nvm_t config;
    config_read(&config);
    offset_x = config.offset_ts_x;
    offset_y = config.offset_ts_y;
}

void thumbstick_calibrate() {
    info("Thumbstick: calibrating...\n");
    float x = 0;
    float y = 0;
    for(uint32_t i=0; i<CFG_CALIBRATION_SAMPLES_THUMBSTICK; i++) {
        if (!(i % CFG_CALIBRATION_BLINK_FREQ)) led_cycle_step();
        x += thumbstick_adc(1, 0.0);
        y += thumbstick_adc(0, 0.0);
    }
    x /= CFG_CALIBRATION_SAMPLES_THUMBSTICK;
    y /= CFG_CALIBRATION_SAMPLES_THUMBSTICK;
    info("Thumbstick: calibration x=%f y=%f\n", x, y);
    config_set_thumbstick_offset(x, y);
    thumbstick_update_offsets();
}

void thumbstick_init() {
    info("INIT: Thumbstick\n");
    adc_init();
    adc_gpio_init(PIN_TX);
    adc_gpio_init(PIN_TY);
    thumbstick_update_offsets();
    thumbstick_update_deadzone();
    // Alternative usage of ABXY while doing daisywheel.
    daisy_a = Button_(PIN_A,  NORMAL, ACTIONS(KEY_NONE));
    daisy_b = Button_(PIN_B,  NORMAL, ACTIONS(KEY_NONE));
    daisy_x = Button_(PIN_X,  NORMAL, ACTIONS(KEY_NONE));
    daisy_y = Button_(PIN_Y,  NORMAL, ACTIONS(KEY_NONE));
}

void thumbstick_report_axis(uint8_t axis, float value) {
    if      (axis == GAMEPAD_AXIS_LX)     hid_gamepad_lx(value);
    else if (axis == GAMEPAD_AXIS_LY)     hid_gamepad_ly(value);
    else if (axis == GAMEPAD_AXIS_RX)     hid_gamepad_rx(value);
    else if (axis == GAMEPAD_AXIS_RY)     hid_gamepad_ry(value);
    else if (axis == GAMEPAD_AXIS_LX_NEG) hid_gamepad_lx(-value);
    else if (axis == GAMEPAD_AXIS_LY_NEG) hid_gamepad_ly(-value);
    else if (axis == GAMEPAD_AXIS_RX_NEG) hid_gamepad_rx(-value);
    else if (axis == GAMEPAD_AXIS_RY_NEG) hid_gamepad_ry(-value);
    else if (axis == GAMEPAD_AXIS_LZ)     hid_gamepad_lz(value);
    else if (axis == GAMEPAD_AXIS_RZ)     hid_gamepad_rz(value);
}

uint8_t thumbstick_get_direction(float angle, float overlap) {
    float a = 45 * (1 - overlap);
    float b = 180 - a;
    uint8_t mask = 0;
    if (is_between(angle, -b, -a)) mask += DIR4_LEFT;
    if (is_between(angle, a, b)) mask += DIR4_RIGHT;
    if (fabs(angle) <= (90 - a)) mask += DIR4_UP;
    if (fabs(angle) >= (90 + a)) mask += DIR4_DOWN;
    return mask;
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
        uint8_t direction = thumbstick_get_direction(pos.angle, self->overlap);
        if (direction & DIR4_LEFT)  self->left.virtual_press = true;
        if (direction & DIR4_RIGHT) self->right.virtual_press = true;
        if (direction & DIR4_UP)    self->up.virtual_press = true;
        if (direction & DIR4_DOWN)  self->down.virtual_press = true;
    }
    // Report directional virtual buttons or axis.
    //// Left.
    if (!hid_is_axis(self->left.actions[0])) self->left.report(&self->left);
    else thumbstick_report_axis(self->left.actions[0], -constrain(pos.x, -1, 0));
    //// Right.
    if (!hid_is_axis(self->right.actions[0])) self->right.report(&self->right);
    else thumbstick_report_axis(self->right.actions[0], constrain(pos.x, 0, 1));
    //// Up.
    if (!hid_is_axis(self->up.actions[0])) self->up.report(&self->up);
    else thumbstick_report_axis(self->up.actions[0], -constrain(pos.y, -1, 0));
    //// Down.
    if (!hid_is_axis(self->down.actions[0])) self->down.report(&self->down);
    else thumbstick_report_axis(self->down.actions[0], constrain(pos.y, 0, 1));
    // Report inner and outer (only if calibrated).
    if (offset_x != 0 && offset_y != 0) {
        self->inner.report(&self->inner);
        self->outer.report(&self->outer);
    }
    // Report push.
    self->push.report(&self->push);
}

void Thumbstick__report_radial(Thumbstick *self, ThumbstickPosition pos) {
    uint8_t direction = thumbstick_get_direction(pos.angle, self->overlap);
    thumbstick_report_axis(self->left.actions[0],  (direction & DIR4_LEFT)  ? pos.radius : 0);
    thumbstick_report_axis(self->right.actions[0], (direction & DIR4_RIGHT) ? pos.radius : 0);
    thumbstick_report_axis(self->up.actions[0],    (direction & DIR4_UP)    ? pos.radius : 0);
    thumbstick_report_axis(self->down.actions[0],  (direction & DIR4_DOWN)  ? pos.radius : 0);
    self->push.report(&self->push);
}

void Thumbstick__config_glyphstick(Thumbstick *self, ...) {
    va_list va;
    va_start(va, 0);
    uint8_t glyph_index = 0;
    uint8_t sub_index = 0;
    uint8_t arg_prev = 0;
    bool action_phase = true;
    // Iterate over provided glyph+actions definition pairs.
    for(uint8_t i=0; true; i++) {
        uint8_t arg = va_arg(va, int);
        if (arg == SENTINEL && arg_prev == SENTINEL) break;
        if (action_phase) {
            // Actions.
            if (arg != SENTINEL) {
                // Store action definition.
                self->glyphstick_actions[glyph_index][sub_index] = arg;
                sub_index += 1;
            } else {
                for(uint8_t j=sub_index; j<4; j++) {
                    // Init all remaining slots to avoid undefined behavior.
                    self->glyphstick_actions[glyph_index][j] = 0;
                }
                sub_index = 0;
                action_phase = false;
            }
        } else {
            // Glyphs.
            if (arg != SENTINEL) {
                // Store glyph definition.
                self->glyphstick_glyphs[glyph_index][sub_index] = arg;
                sub_index += 1;
            } else {
                self->glyphstick_glyphs[glyph_index][sub_index] = SENTINEL;
                for(uint8_t j=sub_index+1; j<8; j++) {
                    // Init all remaining slots to avoid undefined behavior.
                    self->glyphstick_glyphs[glyph_index][j] = 0;
                }
                sub_index = 0;
                glyph_index += 1;
                action_phase = true;
            }
        }
        arg_prev = arg;
    }
    va_end(va);
}

void Thumbstick__report_glyphstick(Thumbstick *self, uint8_t len, Dir4 *input) {
    bool matched = false;
    // Iterate over all defined glyphs.
    for(uint8_t glyph=0; glyph<64; glyph++) {
        // Exit if there is no more glyphs.
        if (self->glyphstick_actions[glyph][0] == 0) break;
        // Pattern match user input against glyph.
        for(uint8_t i=0; i<len; i++) {
            if (input[i] != self->glyphstick_glyphs[glyph][i]) break;
            if (i+1==len && self->glyphstick_glyphs[glyph][i+1] == SENTINEL) {
                hid_press_multiple(self->glyphstick_actions[glyph]);
                hid_release_multiple_later(self->glyphstick_actions[glyph], 100);
                matched = true;
                break;
            }
        }
        if (matched) break;
    }
}

void Thumbstick__config_daisywheel(Thumbstick *self, ...) {
    va_list va;
    va_start(va, 0);
    uint8_t dir_index = 0;
    uint8_t button_index = 0;
    uint8_t action_index = 0;
    // Iterate over the 8 thumbstick directions.
    for(uint8_t i=0; true; i++) {
        if (action_index >= 4) {
            action_index = 0;
            button_index += 1;
        }
        if (button_index >= 4) {
            button_index = 0;
            dir_index += 1;
        }
        if (dir_index >= 8) break;
        uint8_t arg = va_arg(va, int);
        if (arg != SENTINEL) {
            // Store actions based on sequencial indexes.
            self->daisywheel[dir_index][button_index][action_index] = arg;
            action_index += 1;
        } else {
            for(uint8_t j=action_index; j<4; j++) {
                // Init all remaining slots to avoid undefined behavior.
                self->daisywheel[dir_index][button_index][j] = 0;
            }
            action_index = 0;
            button_index += 1;
        }
    }
    va_end(va);
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
    static Dir4 input[8] = {0,};
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
        if (input_index == 0 || dir4 != input[input_index-1]) {
            input[input_index] = dir4;
            input_index += 1;
        }
        // Report daisy keyboard.
        self->report_daisywheel(self, dir8);
    } else {
        if (input_index > 0) {
            // Glyph-stick match.
            if (!daisywheel_used) {
                self->report_glyphstick(self, input_index, input);
            }
            input_index = 0;
            // Daisywheel reset.
            daisywheel_used = false;
            profile_enable_abxy(true);
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
    radius = constrain(radius, 0, 1);
    radius = ramp_low(radius, deadzone);
    x = sin(radians(angle)) * radius;
    y = -cos(radians(angle)) * radius;
    ThumbstickPosition pos = {x, y, angle, radius};
    // Report.
    if (self->mode == THUMBSTICK_MODE_4DIR) self->report_4dir(self, pos, deadzone);
    else if (self->mode == THUMBSTICK_MODE_RADIAL) self->report_radial(self, pos);
    else if (self->mode == THUMBSTICK_MODE_ALPHANUMERIC) self->report_alphanumeric(self, pos);
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
    thumbstick.report_radial = Thumbstick__report_radial;
    thumbstick.report_alphanumeric = Thumbstick__report_alphanumeric;
    thumbstick.reset = Thumbstick__reset;
    thumbstick.config_glyphstick = Thumbstick__config_glyphstick;
    thumbstick.report_glyphstick = Thumbstick__report_glyphstick;
    thumbstick.config_daisywheel = Thumbstick__config_daisywheel;
    thumbstick.report_daisywheel = Thumbstick__report_daisywheel;
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
