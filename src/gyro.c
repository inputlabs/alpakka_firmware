// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include <math.h>
#include <string.h>
#include "button.h"
#include "config.h"
#include "gyro.h"
#include "common.h"
#include "hid.h"
#include "imu.h"
#include "led.h"
#include "pin.h"
#include "touch.h"
#include "vector.h"

double sensitivity_multiplier;

uint8_t world_init = 0;
Vector world_top;
Vector world_fw;
Vector world_right;
Vector accel_smooth;

void gyro_update_sensitivity() {
    uint8_t preset = config_get_mouse_sens_preset();
    sensitivity_multiplier = config_get_mouse_sens_value(preset);
}

void gyro_accel_correction() {
    Vector accel = imu_read_accel();
    // Convert to inverted unit value.
    accel.x /= -BIT_14;
    accel.y /= -BIT_14;
    accel.z /= -BIT_14;
    // Get a smoothed gravity vector.
    accel_smooth = vector_smooth(accel_smooth, accel, CFG_ACCEL_CORRECTION_SMOOTH);
    if (world_init < CFG_ACCEL_CORRECTION_SMOOTH) {
        // It the world space orientation is not fully initialized.
        world_top = vector_normalize(vector_invert(accel_smooth));
        world_fw = vector_cross_product(world_top, (Vector){1, 0, 0});
        world_right = vector_cross_product(world_fw, world_top);
        world_init++;
    } else {
        // Correction.
        float rate_fw = (world_right.z - accel_smooth.x) * CFG_ACCEL_CORRECTION_RATE;
        float rate_r = (world_fw.z - accel_smooth.y) * CFG_ACCEL_CORRECTION_RATE;
        Vector4 correction_fw = quaternion(world_fw, rate_fw);
        Vector4 correction_r = quaternion(world_right, -rate_r);
        Vector4 correction = qmultiply(correction_fw, correction_r);
        world_top = qrotate(correction, world_top);
        world_right = qrotate(correction, world_right);
        world_fw = vector_cross_product(world_top, world_right);
    }
}

void gyro_absolute_output(float value, uint8_t *actions, bool *pressed) {
    for(uint8_t i=0; i<4; i++) {
        uint8_t action = actions[i];
        if (hid_is_axis(action)) {
            value = fabs(value);
            if      (action == GAMEPAD_AXIS_LX)     hid_gamepad_axis(LX,  value);
            else if (action == GAMEPAD_AXIS_LY)     hid_gamepad_axis(LY,  value);
            else if (action == GAMEPAD_AXIS_LZ)     hid_gamepad_axis(LZ,  value);
            else if (action == GAMEPAD_AXIS_RX)     hid_gamepad_axis(RX,  value);
            else if (action == GAMEPAD_AXIS_RY)     hid_gamepad_axis(RY,  value);
            else if (action == GAMEPAD_AXIS_RZ)     hid_gamepad_axis(RZ,  value);
            else if (action == GAMEPAD_AXIS_LX_NEG) hid_gamepad_axis(LX, -value);
            else if (action == GAMEPAD_AXIS_LY_NEG) hid_gamepad_axis(LY, -value);
            else if (action == GAMEPAD_AXIS_LZ_NEG) hid_gamepad_axis(LZ, -value);
            else if (action == GAMEPAD_AXIS_RX_NEG) hid_gamepad_axis(RX, -value);
            else if (action == GAMEPAD_AXIS_RY_NEG) hid_gamepad_axis(RY, -value);
            else if (action == GAMEPAD_AXIS_RZ_NEG) hid_gamepad_axis(RZ, -value);
        } else {
            if (!(*pressed) && value >= 0.5) {
                hid_press(action);
                if (i==3) *pressed = true;
            }
            else if (*pressed && value < 0.5) {
                hid_release(action);
                if (i==3) *pressed = false;
            }
        }
    }
}

void gyro_incremental_output(double value, uint8_t *actions) {
    for(uint8_t i=0; i<4; i++) {
        uint8_t action = actions[i];
        if      (action == MOUSE_X)     hid_mouse_move(value, 0);
        else if (action == MOUSE_Y)     hid_mouse_move(0, value);
        else if (action == MOUSE_X_NEG) hid_mouse_move(-value, 0);
        else if (action == MOUSE_Y_NEG) hid_mouse_move(0, -value);
    }
}

double hssnf(double t, double k, double x) {
    double a = x - (x * k);
    double b = 1 - (x * k * (1/t));
    return a / b;
}

void Gyro__report_absolute(Gyro *self) {
    // Accel-based correction.
    gyro_accel_correction();
    // Get data from gyros.
    Vector gyro = imu_read_gyro();
    static float sens = -BIT_18 * M_PI;
    // Rotate world space orientation.
    Vector4 rx = quaternion(world_right, gyro.y / sens);
    Vector4 ry = quaternion(world_fw, gyro.z / sens);
    Vector4 rz = quaternion(world_top, gyro.x / sens);
    static uint8_t i = 0;
    Vector4 r;
    if      (i==0) r = qmultiply(qmultiply(rx, ry), rz);
    else if (i==1) r = qmultiply(qmultiply(rz, rx), ry);
    else if (i==2) r = qmultiply(qmultiply(ry, rz), rx);
    else if (i==3) r = qmultiply(qmultiply(rx, rz), ry);
    else if (i==4) r = qmultiply(qmultiply(ry, rx), rz);
    else if (i==5) r = qmultiply(qmultiply(rz, ry), rx);
    i++;
    if (i>5) i = 0;
    world_top = qrotate(r, world_top);
    world_fw = qrotate(r, world_fw);
    world_right = vector_cross_product(world_fw, world_top);
    // Debug.
    bool debug = 0;
    if (debug) {
        hid_gamepad_axis(LX, world_top.x);
        hid_gamepad_axis(LY, -world_top.y);
        hid_gamepad_axis(RX, world_fw.x);
        hid_gamepad_axis(RY, -world_fw.y);
        return;
    }
    // Output calculation.
    float x = degrees(asin(-world_right.z)) / 90;
    float y = degrees(asin(-world_top.z)) / 90;
    float z = degrees(asin(world_fw.z)) / 90;
    if (fabs(x) > 0.5 && z < 0) x += -z * 2 * sign(x); // Steering lock.
    x = constrain(x * 1.1, -1, 1); // Additional saturation.
    x = ramp(x, self->absolute_x_min/90, self->absolute_x_max/90); // Adjust range.
    y = ramp(y, self->absolute_y_min/90, self->absolute_y_max/90); // Adjust range.
    // Output mapping.
    if (x >= 0) gyro_absolute_output( x, self->actions_x_pos, &(self->pressed_x_pos));
    else        gyro_absolute_output(-x, self->actions_x_neg, &(self->pressed_x_neg));
    if (y >= 0) gyro_absolute_output( y, self->actions_y_pos, &(self->pressed_y_pos));
    else        gyro_absolute_output(-y, self->actions_y_neg, &(self->pressed_y_neg));
    // printf("\r%6.1f %6.1f %6.1f", x*100, y*100, z*100);
}

void Gyro__report_incremental(Gyro *self) {
    static double sub_x = 0;
    static double sub_y = 0;
    static double sub_z = 0;
     // Read gyro values.
    Vector imu_gyro = imu_read_gyro();
    double x = imu_gyro.x * CFG_GYRO_SENSITIVITY_X * sensitivity_multiplier;
    double y = imu_gyro.y * CFG_GYRO_SENSITIVITY_Y * sensitivity_multiplier;
    double z = imu_gyro.z * CFG_GYRO_SENSITIVITY_Z * sensitivity_multiplier;
    // Additional processing.
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
    if (x >= 0) gyro_incremental_output( x, self->actions_x_pos);
    else        gyro_incremental_output(-x, self->actions_x_neg);
    if (y >= 0) gyro_incremental_output( y, self->actions_y_pos);
    else        gyro_incremental_output(-y, self->actions_y_neg);
    if (z >= 0) gyro_incremental_output( z, self->actions_z_pos);
    else        gyro_incremental_output(-z, self->actions_z_neg);
}

bool Gyro__is_engaged(Gyro *self) {
    if (self->engage == PIN_NONE) return false;
    if (self->engage == PIN_TOUCH_IN) return touch_status();
    return self->engage_button.is_pressed(&(self->engage_button));
}

void Gyro__report(Gyro *self) {
    if (self->mode == GYRO_MODE_TOUCH_ON) {
        if (self->is_engaged(self)) self->report_incremental(self);
    }
    else if (self->mode == GYRO_MODE_TOUCH_OFF) {
        if (!self->is_engaged(self)) self->report_incremental(self);
    }
    else if (self->mode == GYRO_MODE_ALWAYS_ON) {
        self->report_incremental(self);
    }
    else if (self->mode == GYRO_MODE_AXIS_ABSOLUTE) {
        self->report_absolute(self);
    }
    else if (self->mode == GYRO_MODE_OFF) {
        return;
    }
}

void Gyro__reset(Gyro *self) {
    world_init = 0;
    self->pressed_x_pos = false;
    self->pressed_y_pos = false;
    self->pressed_z_pos = false;
    self->pressed_x_neg = false;
    self->pressed_y_neg = false;
    self->pressed_z_neg = false;
}

void Gyro__config_x(Gyro *self, double min, double max, Actions neg, Actions pos) {
    self->absolute_x_min = min;
    self->absolute_x_max = max;
    memcpy(self->actions_x_neg, neg, ACTIONS_LEN);
    memcpy(self->actions_x_pos, pos, ACTIONS_LEN);
}

void Gyro__config_y(Gyro *self, double min, double max, Actions neg, Actions pos) {
    self->absolute_y_min = min;
    self->absolute_y_max = max;
    memcpy(self->actions_y_neg, neg, ACTIONS_LEN);
    memcpy(self->actions_y_pos, pos, ACTIONS_LEN);
}

void Gyro__config_z(Gyro *self, double min, double max, Actions neg, Actions pos) {
    self->absolute_z_min = min;
    self->absolute_z_max = max;
    memcpy(self->actions_z_neg, neg, ACTIONS_LEN);
    memcpy(self->actions_z_pos, pos, ACTIONS_LEN);
}

Gyro Gyro_ (
    GyroMode mode,
    uint8_t engage
) {
    Gyro gyro;
    gyro.is_engaged = Gyro__is_engaged;
    gyro.report = Gyro__report;
    gyro.report_incremental = Gyro__report_incremental;
    gyro.report_absolute = Gyro__report_absolute;
    gyro.reset = Gyro__reset;
    gyro.config_x = Gyro__config_x;
    gyro.config_y = Gyro__config_y;
    gyro.config_z = Gyro__config_z;
    gyro.mode = mode;
    gyro.engage = engage;
    if (engage != PIN_NONE && engage != PIN_TOUCH_IN) {
        Actions none = {0,};
        gyro.engage_button = Button_(engage, NORMAL, none, none, none);
    }
    memset(gyro.actions_x_pos, 0, ACTIONS_LEN);
    memset(gyro.actions_y_pos, 0, ACTIONS_LEN);
    memset(gyro.actions_z_pos, 0, ACTIONS_LEN);
    memset(gyro.actions_x_neg, 0, ACTIONS_LEN);
    memset(gyro.actions_y_neg, 0, ACTIONS_LEN);
    memset(gyro.actions_z_neg, 0, ACTIONS_LEN);
    gyro_update_sensitivity();
    gyro.reset(&gyro);
    return gyro;
}
