// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include <math.h>
#include <pico/stdlib.h>
#include <hardware/adc.h>
#include "config.h"
#include "pin.h"
#include "button.h"
#include "thumbstick.h"
#include "helper.h"
#include "hid.h"
#include "led.h"

float offset_x = 0;
float offset_y = 0;
float deadzone = 0;

float thumbstick_value(uint8_t adc_index, float offset) {
    adc_select_input(adc_index);
    float value = (float)adc_read() - 2048;
    value = value / 2048 * CFG_THUMBSTICK_SATURATION;
    return limit_between(value - offset, -1, 1);
}

Thumbstick_8dir thumbstick_get_8dir(Thumbstick_position pos) {
    if (pos.radius <= deadzone) return DIR8_CENTER;
    if      (pos.angle < -ANGLE_CUT_4) return DIR8_DOWN;
    else if (pos.angle < -ANGLE_CUT_3) return DIR8_DOWN_LEFT;
    else if (pos.angle < -ANGLE_CUT_2) return DIR8_LEFT;
    else if (pos.angle < -ANGLE_CUT_1) return DIR8_UP_LEFT;
    else if (pos.angle <  ANGLE_CUT_1) return DIR8_UP;
    else if (pos.angle <  ANGLE_CUT_2) return DIR8_UP_RIGHT;
    else if (pos.angle <  ANGLE_CUT_3) return DIR8_RIGHT;
    else if (pos.angle <  ANGLE_CUT_4) return DIR8_DOWN_RIGHT;
    else if (pos.angle >  ANGLE_CUT_4) return DIR8_DOWN;
}

void thumbstick_update_deadzone() {
    config_nvm_t config;
    config_read(&config);
    float deadzones[3] = {
        CFG_THUMBSTICK_DEADZONE_LOW,
        CFG_THUMBSTICK_DEADZONE_MID,
        CFG_THUMBSTICK_DEADZONE_HIGH
    };
    deadzone = deadzones[config.deadzone];
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
        x += thumbstick_value(1, 0.0);
        y += thumbstick_value(0, 0.0);
    }
    x /= len;
    y /= len;
    printf("Thumbstick: calibration x=%f y=%f\n", x, y);
    config_set_thumbstick_offset(x, y);
    thumbstick_update_offsets();
}

void thumbstick_init() {
    printf("Thumbstick: Init...\n");
    adc_init();
    adc_gpio_init(PIN_TX);
    adc_gpio_init(PIN_TY);
    thumbstick_update_offsets();
    thumbstick_update_deadzone();
    printf("Thumbstick: Init OK\n");
}

void Thumbstick__report(Thumbstick *self) {
    // Calculate trigonometry.
    float x = thumbstick_value(1, offset_x);
    float y = thumbstick_value(0, offset_y);
    float angle = atan2(x, -y) * (180 / M_PI);
    float radius = sqrt(powf(x, 2) + powf(y, 2));
    radius = limit_between(radius, 0, 1);
    radius = ramp_low(radius, deadzone);
    x = sin(radians(angle)) * radius;
    y = -cos(radians(angle)) * radius;
    Thumbstick_position position = {x, y, angle, radius};
    // Report.
    if (self->mode == THUMBSTICK_MODE_4DIR) {
        self->report_4dir(self, position);
    }
    if (self->mode == THUMBSTICK_MODE_KEYBOARD) {
        self->report_keyboard(self, position);
    }
}

void Thumbstick__report_4dir(Thumbstick *self, Thumbstick_position pos) {
    // Evaluate virtual buttons.
    if (pos.radius > deadzone) {
        if (pos.radius < CFG_THUMBSTICK_INNER_RADIUS) self->inner.virtual_press = true;
        else self->outer.virtual_press = true;
        if (is_between(pos.angle, -157.5, -22.5)) self->left.virtual_press = true;
        if (is_between(pos.angle, 22.5, 157.5)) self->right.virtual_press = true;
        if (fabs(pos.angle) < 67.5) self->up.virtual_press = true;
        if (fabs(pos.angle) > 112.5) self->down.virtual_press = true;
    }
    // Report directional virtual buttons.
    if (self->left.actions[0]  < GAMEPAD_AXIS_INDEX) self->left.report(&self->left);
    if (self->right.actions[0] < GAMEPAD_AXIS_INDEX) self->right.report(&self->right);
    if (self->up.actions[0]    < GAMEPAD_AXIS_INDEX) self->up.report(&self->up);
    if (self->down.actions[0]  < GAMEPAD_AXIS_INDEX) self->down.report(&self->down);
    // Report inner and outer (only if calibrated).
    if (offset_x != 0 && offset_y != 0) {
        self->inner.report(&self->inner);
        self->outer.report(&self->outer);
    }
    // Report push.
    self->push.report(&self->push);
    // Report axis.
    if (pos.x < 0) {
        if (self->left.actions[0] == GAMEPAD_AXIS_LX)     hid_gamepad_lx(-pos.x * ANALOG_FACTOR);
        if (self->left.actions[0] == GAMEPAD_AXIS_LY)     hid_gamepad_ly( pos.x * ANALOG_FACTOR);
        if (self->left.actions[0] == GAMEPAD_AXIS_RX)     hid_gamepad_rx(-pos.x * ANALOG_FACTOR);
        if (self->left.actions[0] == GAMEPAD_AXIS_RY)     hid_gamepad_ry( pos.x * ANALOG_FACTOR);
        if (self->left.actions[0] == GAMEPAD_AXIS_LX_NEG) hid_gamepad_lx( pos.x * ANALOG_FACTOR);
        if (self->left.actions[0] == GAMEPAD_AXIS_LY_NEG) hid_gamepad_ly(-pos.x * ANALOG_FACTOR);
        if (self->left.actions[0] == GAMEPAD_AXIS_RX_NEG) hid_gamepad_rx( pos.x * ANALOG_FACTOR);
        if (self->left.actions[0] == GAMEPAD_AXIS_RY_NEG) hid_gamepad_ry(-pos.x * ANALOG_FACTOR);
        if (self->left.actions[0] == GAMEPAD_AXIS_LZ)     hid_gamepad_lz(max(0, -pos.x) * TRIGGER_FACTOR);
        if (self->left.actions[0] == GAMEPAD_AXIS_RZ)     hid_gamepad_rz(max(0, -pos.x) * TRIGGER_FACTOR);
    }
    if (pos.x > 0) {
        if (self->right.actions[0] == GAMEPAD_AXIS_LX)     hid_gamepad_lx( pos.x * ANALOG_FACTOR);
        if (self->right.actions[0] == GAMEPAD_AXIS_LY)     hid_gamepad_ly(-pos.x * ANALOG_FACTOR);
        if (self->right.actions[0] == GAMEPAD_AXIS_RX)     hid_gamepad_rx( pos.x * ANALOG_FACTOR);
        if (self->right.actions[0] == GAMEPAD_AXIS_RY)     hid_gamepad_ry(-pos.x * ANALOG_FACTOR);
        if (self->right.actions[0] == GAMEPAD_AXIS_LX_NEG) hid_gamepad_lx(-pos.x * ANALOG_FACTOR);
        if (self->right.actions[0] == GAMEPAD_AXIS_LY_NEG) hid_gamepad_ly( pos.x * ANALOG_FACTOR);
        if (self->right.actions[0] == GAMEPAD_AXIS_RX_NEG) hid_gamepad_rx(-pos.x * ANALOG_FACTOR);
        if (self->right.actions[0] == GAMEPAD_AXIS_RY_NEG) hid_gamepad_ry( pos.x * ANALOG_FACTOR);
        if (self->right.actions[0] == GAMEPAD_AXIS_LZ)     hid_gamepad_lz(max(0, pos.x) * TRIGGER_FACTOR);
        if (self->right.actions[0] == GAMEPAD_AXIS_RZ)     hid_gamepad_rz(max(0, pos.x) * TRIGGER_FACTOR);
    }
    if (pos.y < 0) {
        if (self->up.actions[0] == GAMEPAD_AXIS_LX)     hid_gamepad_lx(-pos.y * ANALOG_FACTOR);
        if (self->up.actions[0] == GAMEPAD_AXIS_LY)     hid_gamepad_ly( pos.y * ANALOG_FACTOR);
        if (self->up.actions[0] == GAMEPAD_AXIS_RX)     hid_gamepad_rx(-pos.y * ANALOG_FACTOR);
        if (self->up.actions[0] == GAMEPAD_AXIS_RY)     hid_gamepad_ry( pos.y * ANALOG_FACTOR);
        if (self->up.actions[0] == GAMEPAD_AXIS_LX_NEG) hid_gamepad_lx( pos.y * ANALOG_FACTOR);
        if (self->up.actions[0] == GAMEPAD_AXIS_LY_NEG) hid_gamepad_ly(-pos.y * ANALOG_FACTOR);
        if (self->up.actions[0] == GAMEPAD_AXIS_RX_NEG) hid_gamepad_rx( pos.y * ANALOG_FACTOR);
        if (self->up.actions[0] == GAMEPAD_AXIS_RY_NEG) hid_gamepad_ry(-pos.y * ANALOG_FACTOR);
        if (self->up.actions[0] == GAMEPAD_AXIS_LZ)     hid_gamepad_lz(max(0, -pos.y) * TRIGGER_FACTOR);
        if (self->up.actions[0] == GAMEPAD_AXIS_RZ)     hid_gamepad_rz(max(0, -pos.y) * TRIGGER_FACTOR);
    }
    if (pos.y > 0) {
        if (self->down.actions[0] == GAMEPAD_AXIS_LX)     hid_gamepad_lx( pos.y * ANALOG_FACTOR);
        if (self->down.actions[0] == GAMEPAD_AXIS_LY)     hid_gamepad_ly(-pos.y * ANALOG_FACTOR);
        if (self->down.actions[0] == GAMEPAD_AXIS_RX)     hid_gamepad_rx( pos.y * ANALOG_FACTOR);
        if (self->down.actions[0] == GAMEPAD_AXIS_RY)     hid_gamepad_ry(-pos.y * ANALOG_FACTOR);
        if (self->down.actions[0] == GAMEPAD_AXIS_LX_NEG) hid_gamepad_lx(-pos.y * ANALOG_FACTOR);
        if (self->down.actions[0] == GAMEPAD_AXIS_LY_NEG) hid_gamepad_ly( pos.y * ANALOG_FACTOR);
        if (self->down.actions[0] == GAMEPAD_AXIS_RX_NEG) hid_gamepad_rx(-pos.y * ANALOG_FACTOR);
        if (self->down.actions[0] == GAMEPAD_AXIS_RY_NEG) hid_gamepad_ry( pos.y * ANALOG_FACTOR);
        if (self->down.actions[0] == GAMEPAD_AXIS_LZ)     hid_gamepad_lz(max(0, pos.y) * TRIGGER_FACTOR);
        if (self->down.actions[0] == GAMEPAD_AXIS_RZ)     hid_gamepad_rz(max(0, pos.y) * TRIGGER_FACTOR);
    }
}

void Thumbstick__report_keyboard(Thumbstick *self, Thumbstick_position pos) {
    Thumbstick_8dir direction = thumbstick_get_8dir(pos);
    if (pos.radius > 0.10) {
        if (pos.radius > 0.90) {
            if (direction == DIR8_UP) {
                self->up.virtual_press = false;
                self->up.virtual_press_secondary = true;
            }
            if (direction == DIR8_RIGHT) {
                self->right.virtual_press = false;
                self->right.virtual_press_secondary = true;
            }
        } else {
            if (direction == DIR8_UP) {
                self->up.virtual_press = true;
                self->up.virtual_press_secondary = false;
            }
            if (direction == DIR8_RIGHT) {
                self->right.virtual_press = true;
                self->right.virtual_press_secondary = false;
            }
        }
    } else {
        self->up.reset(&self->up);
        self->right.reset(&self->right);
    }
    self->up.report(&self->up);
    self->right.report(&self->right);
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
    Thumbstick_mode mode,
    Button left,
    Button right,
    Button up,
    Button down,
    Button corner_ul,
    Button corner_ur,
    Button corner_dl,
    Button corner_dr,
    Button push,
    Button inner,
    Button outer
) {
    Thumbstick thumbstick;
    thumbstick.report = Thumbstick__report;
    thumbstick.report_4dir = Thumbstick__report_4dir;
    thumbstick.report_keyboard = Thumbstick__report_keyboard;
    thumbstick.reset = Thumbstick__reset;
    thumbstick.mode = mode;
    thumbstick.left = left;
    thumbstick.right = right;
    thumbstick.up = up;
    thumbstick.down = down;
    thumbstick.corner_ul = corner_ul;
    thumbstick.corner_ur = corner_ur;
    thumbstick.corner_dl = corner_dl;
    thumbstick.corner_dr = corner_dr;
    thumbstick.push = push;
    thumbstick.inner = inner;
    thumbstick.outer = outer;
    return thumbstick;
}
