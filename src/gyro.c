// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include <stdarg.h>
#include "config.h"
#include "pin.h"
#include "button.h"
#include "gyro.h"
#include "imu.h"
#include "hid.h"
#include "touch.h"

bool Gyro__is_engaged(Gyro *self) {
    if (self->pin == PIN_NONE) return false;
    if (self->pin == PIN_TOUCH_IN) return touch_status();
    return self->engage_button.is_pressed(&(self->engage_button));
}

void Gyro__report(Gyro *self) {
    // Mode
    if (self->mode == GYRO_MODE_TOUCH_ON) {
        if (self->is_engaged(self) == false) return;
    }
    else if (self->mode == GYRO_MODE_TOUCH_OFF) {
        if (self->is_engaged(self) == true) return;
    }
    else if (self->mode == GYRO_MODE_ALWAYS_OFF) {
        return;
    }
    // Report.
    vector_t imu_gyro = imu_read_gyro();
    int16_t x = (int16_t)imu_gyro.x;
    int16_t y = (int16_t)imu_gyro.y;
    int16_t z = (int16_t)imu_gyro.z;
    for(uint8_t i=0; i<4; i++) {
        uint8_t action = self->actions_x[i];
        if (action == MOUSE_X) hid_mouse_move(x, 0);
        else if (action == MOUSE_Y) hid_mouse_move(0, x);
        else if (action == MOUSE_X_NEG) hid_mouse_move(-x, 0);
        else if (action == MOUSE_Y_NEG) hid_mouse_move(0, -x);
    }
    for(uint8_t i=0; i<4; i++) {
        uint8_t action = self->actions_y[i];
        if (action == MOUSE_X) hid_mouse_move(y, 0);
        else if (action == MOUSE_Y) hid_mouse_move(0, y);
        else if (action == MOUSE_X_NEG) hid_mouse_move(-y, 0);
        else if (action == MOUSE_Y_NEG) hid_mouse_move(0, -y);
    }
    for(uint8_t i=0; i<4; i++) {
        uint8_t action = self->actions_z[i];
        if (action == MOUSE_X) hid_mouse_move(z, 0);
        else if (action == MOUSE_Y) hid_mouse_move(0, z);
        else if (action == MOUSE_X_NEG) hid_mouse_move(-z, 0);
        else if (action == MOUSE_Y_NEG) hid_mouse_move(0, -z);
    }
}

void Gyro__reset(Gyro *self) {
}

Gyro Gyro_ (
    uint8_t mode,
    uint8_t pin,
    ...  // Actions
) {
    Gyro gyro;
    gyro.is_engaged = Gyro__is_engaged;
    gyro.report = Gyro__report;
    gyro.reset = Gyro__reset;
    gyro.mode = mode;
    gyro.pin = pin;
    if (pin != PIN_TOUCH_IN) {
        gyro.engage_button = Button_(pin, NORMAL, ACTIONS(KEY_NONE));
    }
    gyro.actions_x[0] = 0;
    gyro.actions_x[1] = 0;
    gyro.actions_x[2] = 0;
    gyro.actions_x[3] = 0;
    gyro.actions_y[0] = 0;
    gyro.actions_y[1] = 0;
    gyro.actions_y[2] = 0;
    gyro.actions_y[3] = 0;
    gyro.actions_z[0] = 0;
    gyro.actions_z[1] = 0;
    gyro.actions_z[2] = 0;
    gyro.actions_z[3] = 0;
    va_list va;
    va_start(va, 0);
    for(uint8_t i=0; true; i++) {
        uint8_t value = va_arg(va, int);
        if (value == SENTINEL) break;
        gyro.actions_x[i] = value;
    }
    for(uint8_t i=0; true; i++) {
        uint8_t value = va_arg(va, int);
        if (value == SENTINEL) break;
        gyro.actions_y[i] = value;
    }
    for(uint8_t i=0; true; i++) {
        uint8_t value = va_arg(va, int);
        if (value == SENTINEL) break;
        gyro.actions_z[i] = value;
    }
    va_end(va);
    return gyro;
}
