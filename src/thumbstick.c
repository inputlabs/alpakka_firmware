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

Thumbstick_8dir thumbstick_get_8dir(Thumbstick_position pos, float deadzone) {
    if (pos.radius <= deadzone) return DIR8_CENTER;
    else if (pos.angle < -ANGLE_CUT_4) return DIR8_DOWN;
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
    static Thumbstick_8dir direction_prev = DIR8_CENTER;
    static Thumbstick_8dir engaged = false;
    static bool outer_prev = false;
    static uint8_t preview = 0;
    static uint8_t index_last = -1;
    static Button button;

    Thumbstick_8dir direction = thumbstick_get_8dir(pos, 0.50);
    bool outer = pos.radius > 0.51;

    // SKIP REPEATED
    if (direction == direction_prev && outer == outer_prev) return;

    // ACCEPT
    if (direction == DIR8_CENTER) {
        preview = 0;
        index_last = -1;
        engaged = false;
    }

    // ENGAGE
    if (outer && !engaged) {
        if      (direction == DIR8_UP)         button = self->up;
        else if (direction == DIR8_UP_RIGHT)   button = self->up_right;
        else if (direction == DIR8_RIGHT)      button = self->right;
        else if (direction == DIR8_DOWN_RIGHT) button = self->down_right;
        else if (direction == DIR8_DOWN)       button = self->down;
        else if (direction == DIR8_DOWN_LEFT)  button = self->down_left;
        else if (direction == DIR8_LEFT)       button = self->left;
        else if (direction == DIR8_UP_LEFT)    button = self->up_left;
        engaged = direction;
    }

    // DIAL
    if (outer && engaged) {
        // int8_t index = direction - engaged;
        // uint8_t mod = button.actions[3] != 0 ? 4 : 3;
        // index = (index % 8) / 2;
        int8_t index = index_last;
        if (engaged == DIR8_UP) {
            if (direction == DIR8_UP)    index = 0;
            if (direction == DIR8_RIGHT) index = 1;
            if (direction == DIR8_DOWN)  index = 2;
            if (direction == DIR8_LEFT)  index = 3;
        }
        else if (engaged == DIR8_RIGHT) {
            if (direction == DIR8_UP)    index = 2;
            if (direction == DIR8_RIGHT) index = 0;
            if (direction == DIR8_DOWN)  index = 1;
            if (direction == DIR8_LEFT)  index = 2;
        }
        else if (engaged == DIR8_DOWN) {
            if (direction == DIR8_UP)    index = 2;
            if (direction == DIR8_RIGHT) index = 2;
            if (direction == DIR8_DOWN)  index = 0;
            if (direction == DIR8_LEFT)  index = 1;
        }
        else if (engaged == DIR8_LEFT) {
            if (direction == DIR8_UP)    index = 1;
            if (direction == DIR8_RIGHT) index = 2;
            if (direction == DIR8_DOWN)  index = 3;
            if (direction == DIR8_LEFT)  index = 0;
        }
        else if (engaged == DIR8_UP_RIGHT) {
            if (direction == DIR8_UP_RIGHT)   index = 0;
            if (direction == DIR8_DOWN_RIGHT) index = 1;
            if (direction == DIR8_DOWN_LEFT)  index = 2;
            if (direction == DIR8_UP_LEFT)    index = 3;
        }
        else if (engaged == DIR8_DOWN_RIGHT) {
            if (direction == DIR8_UP_RIGHT)   index = 3;
            if (direction == DIR8_DOWN_RIGHT) index = 0;
            if (direction == DIR8_DOWN_LEFT)  index = 1;
            if (direction == DIR8_UP_LEFT)    index = 2;
        }
        else if (engaged == DIR8_DOWN_LEFT) {
            if (direction == DIR8_UP_RIGHT)   index = 2;
            if (direction == DIR8_DOWN_RIGHT) index = 3;
            if (direction == DIR8_DOWN_LEFT)  index = 0;
            if (direction == DIR8_UP_LEFT)    index = 1;
        }
        else if (engaged == DIR8_UP_LEFT) {
            if (direction == DIR8_UP_RIGHT)   index = 1;
            if (direction == DIR8_DOWN_RIGHT) index = 2;
            if (direction == DIR8_DOWN_LEFT)  index = 3;
            if (direction == DIR8_UP_LEFT)    index = 0;
        }
        if (index != index_last) {
            if (preview) {
                hid_press_and_release(KEY_BACKSPACE, 10);
                preview = 0;
            }
            add_alarm_in_ms(
                20,
                (alarm_callback_t)hid_press_delayed,
                (void*)(uint32_t)button.actions[index],
                true
            );
            add_alarm_in_ms(
                30,
                (alarm_callback_t)hid_release_delayed,
                (void*)(uint32_t)button.actions[index],
                true
            );
            index_last = index;
            preview = 1;
        }
    }
    direction_prev = direction;
    outer_prev = outer;
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
    Button up_left,
    Button up_right,
    Button down_left,
    Button down_right,
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
    thumbstick.up_left = up_left;
    thumbstick.up_right = up_right;
    thumbstick.down_left = down_left;
    thumbstick.down_right = down_right;
    thumbstick.push = push;
    thumbstick.inner = inner;
    thumbstick.outer = outer;
    return thumbstick;
}
