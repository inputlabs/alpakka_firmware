// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include <stdarg.h>
#include "config.h"
#include "pin.h"
#include "profile.h"
#include "button.h"
#include "gyro.h"
#include "imu.h"
#include "hid.h"
#include "touch.h"
#include "helper.h"
#include "led.h"

double antideadzone = 0; // TODO ALPHA
double absx = 0;  // TODO ALPHA

void gyro_antideadzone(int8_t increment) {
    if (increment > 0) antideadzone += 0.05;
    else antideadzone -= 0.05;
    antideadzone = limit_between(antideadzone, 0, 0.50);
    printf("antideadzone=%f\n", antideadzone);
    uint8_t adz = (antideadzone * 100) + 0.001;
    led_shape_all_off();
    if      (adz == 5)  led_blink_mask(0b0001);
    else if (adz == 10) led_blink_mask(0b0010);
    else if (adz == 15) led_blink_mask(0b0100);
    else if (adz == 20) led_blink_mask(0b1000);
    else if (adz == 25) led_blink_mask(0b0011);
    else if (adz == 30) led_blink_mask(0b0110);
    else if (adz == 35) led_blink_mask(0b1100);
    else if (adz == 40) led_blink_mask(0b1001);
    else if (adz == 45) led_blink_mask(0b1011);
    else if (adz == 50) led_blink_mask(0b0111);
}

void gyro_correct_(double value) {
    static double threshold = 5000;
    static double speed = 500;
    // printf("\r%6.0f %6.0f", absx, value);
    if (fabs(value) > threshold) return;
    double delta = value - absx;
    absx += (delta / speed);
}

void gyro_correct() {
    static double smooth = 0;
    static double s = 20;
    vector_t accel = imu_read_accel();
    smooth = ((smooth * (s-1)) + accel.x) / s;
    double final = limit_between(smooth * 32768, -32767, 32767);
    gyro_correct_(final);
}

void Gyro__report_absolute(Gyro *self) {
    vector_t gyro = imu_read_gyro_alt();
    absx += gyro.x * 10;
    // printf("\r%6.0f %6.0f", absx), finalx;
    double finalx = limit_between(absx, -32767, 32767);
    finalx = finalx > 0 ? ramp_inv(finalx, antideadzone, 32767) : -ramp_inv(-finalx, antideadzone, 32767);
    gyro_correct();
    hid_gamepad_lx(finalx);
    // printf("\r%7.0f\n", finalx);
    // if (self->engage_button.is_pressed(&(self->engage_button))) {
    //     absx = 0;
    //     // printf("RECENTER\n");
    // }
}

bool Gyro__is_engaged(Gyro *self) {
    if (self->pin == PIN_NONE) return false;
    if (self->pin == PIN_TOUCH_IN) return touch_status();
    return self->engage_button.is_pressed(&(self->engage_button));
}

void Gyro__report(Gyro *self) {
    // Mode
    if (self->mode == GYRO_MODE_TOUCH_ON) {
        if (!self->is_engaged(self)) return;
    }
    else if (self->mode == GYRO_MODE_TOUCH_OFF) {
        if (self->is_engaged(self)) return;
    }
    else if (self->mode == GYRO_MODE_ALWAYS_OFF) {
        return;
    }
    else if (self->mode == GYRO_MODE_AXIS_RELATIVE) {
        self->report_absolute(self);
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
    GyroMode mode,
    uint8_t pin,
    ...  // Actions
) {
    Gyro gyro;
    gyro.is_engaged = Gyro__is_engaged;
    gyro.report = Gyro__report;
    gyro.report_absolute = Gyro__report_absolute;
    gyro.reset = Gyro__reset;
    gyro.mode = mode;
    gyro.pin = pin;
    if (pin != PIN_NONE && pin != PIN_TOUCH_IN) {
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
