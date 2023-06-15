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

double sensitivity_multiplier;
double antideadzone = 0; // TODO ALPHA
double absx = 0;  // TODO ALPHA

void gyro_update_sensitivity() {
    config_nvm_t config;
    config_read(&config);
    float multipliers[3] = {
        CFG_GYRO_SENSITIVITY_MULTIPLIER_LOW,
        CFG_GYRO_SENSITIVITY_MULTIPLIER_MID,
        CFG_GYRO_SENSITIVITY_MULTIPLIER_HIGH
    };
    sensitivity_multiplier = multipliers[config.sensitivity];
}

void gyro_wheel_antideadzone(int8_t increment) {
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

void gyro_wheel_recenter() {
    static double smoothing = 20;
    static double threshold = 0.07;
    static double speed = 500;
    static double smoothed = 0;
    vector_t accel = imu_read_accel();
    double x = accel.x / BIT_14;
    smoothed = ((smoothed * (smoothing-1)) + x) / smoothing;
    double correction = limit_between(smoothed, -1, 1);
    if (fabs(correction) > threshold) return;
    double delta = correction - absx;
    absx += (delta / speed);
}

double hssnf(double t, double k, double x) {
    double a = x - (x * k);
    double b = 1 - (x * k * (1/t));
    return a / b;
}

void Gyro__report_absolute(Gyro *self) {
    vector_t gyro = imu_read_gyro();
    absx += (gyro.x * CFG_GYRO_SENSITIVITY_X) / 3500;
    gyro_wheel_recenter();
    double x = limit_between(absx, -1, 1);
    x = x > 0 ? ramp_inv(x, antideadzone) : -ramp_inv(-x, antideadzone);
    hid_gamepad_lx(x);
}

void Gyro__report_incremental(Gyro *self) {
    static double sub_x = 0;
    static double sub_y = 0;
    static double sub_z = 0;
     // Read gyro values.
    vector_t imu_gyro = imu_read_gyro();
    double x = imu_gyro.x * CFG_GYRO_SENSITIVITY_X * sensitivity_multiplier;
    double y = imu_gyro.y * CFG_GYRO_SENSITIVITY_Y * sensitivity_multiplier;
    double z = imu_gyro.z * CFG_GYRO_SENSITIVITY_Z * sensitivity_multiplier;
    // Magic happens.
    double t = 1.0;
    double k = 0.5;
    if      (x > 0 && x <  t) x =  hssnf(t, k,  x);
    else if (x < 0 && x > -t) x = -hssnf(t, k, -x);
    if      (y > 0 && y <  t) y =  hssnf(t, k,  y);
    else if (y < 0 && y > -t) y = -hssnf(t, k, -y);
    if      (z > 0 && z <  t) z =  hssnf(t, k,  z);
    else if (z < 0 && z > -t) z = -hssnf(t, k, -z);
    // Reintroduce subpixel leftovers.
    x += sub_x;
    y += sub_y;
    z += sub_z;
    // Round down and save leftovers.
    sub_x = modf(x, &x);
    sub_y = modf(y, &y);
    sub_z = modf(z, &z);
    // Report.
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

bool Gyro__is_engaged(Gyro *self) {
    if (self->pin == PIN_NONE) return false;
    if (self->pin == PIN_TOUCH_IN) return touch_status();
    return self->engage_button.is_pressed(&(self->engage_button));
}

void Gyro__report(Gyro *self) {
    if (self->mode == GYRO_MODE_TOUCH_ON) {
        if (self->is_engaged(self)) self->report_incremental(self);
    }
    else if (self->mode == GYRO_MODE_TOUCH_OFF) {
        if (!self->is_engaged(self)) self->report_incremental(self);
    }
    else if (self->mode == GYRO_MODE_AXIS_ABSOLUTE) {
        self->report_absolute(self);
    }
    else if (self->mode == GYRO_MODE_ALWAYS_OFF) {
        return;
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
    gyro.report_incremental = Gyro__report_incremental;
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
    gyro_update_sensitivity();
    return gyro;
}
