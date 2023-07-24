// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include <stdarg.h>
#include <math.h>
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
bool init = false;

// double absx = 0;  // TODO ALPHA

vector_t world_top;
vector_t world_fw;
vector_t world_right;

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
    if (increment > 0) antideadzone += 0.01;
    else antideadzone -= 0.01;
    antideadzone = constrain(antideadzone, 0, 0.50);
    printf("antideadzone=%f\n", antideadzone);
    uint8_t adz = (antideadzone * 100) + 0.001;
    led_shape_all_off();
    if      (adz==1  || adz==11 || adz==21) led_blink_mask(0b0001);
    else if (adz==2  || adz==12 || adz==22) led_blink_mask(0b0010);
    else if (adz==3  || adz==13 || adz==23) led_blink_mask(0b0100);
    else if (adz==4  || adz==14 || adz==24) led_blink_mask(0b1000);
    else if (adz==5  || adz==15 || adz==25) led_blink_mask(0b0011);
    else if (adz==6  || adz==16 || adz==26) led_blink_mask(0b0110);
    else if (adz==7  || adz==17 || adz==27) led_blink_mask(0b1100);
    else if (adz==8  || adz==18 || adz==28) led_blink_mask(0b1001);
    else if (adz==9  || adz==19 || adz==29) led_blink_mask(0b1011);
    else if (adz==10 || adz==20 || adz==30) led_blink_mask(0b0111);
}

vector_t normalize(vector_t v) {
    double mag = (v.x*v.x) + (v.y*v.y) + (v.z*v.z);
    if (fabs(mag - 1.0) > 0.0001) {  // Tolerance.
        mag = sqrt(mag);
        return (vector_t){v.x/mag, v.y/mag, v.z/mag};
    }
    return v;
}

vector_t cross_product(vector_t a, vector_t b) {
    return (vector_t){
        (a.y * b.z) - (a.z * b.y),
        (a.z * b.x) - (a.x * b.z),
        (a.x * b.y) - (a.y * b.x)
    };
}

vector_t vector_add(vector_t a, vector_t b) {
    return (vector_t){a.x+b.x, a.y+b.y, a.z+b.z};
}

vector_t vector_multiply(vector_t v, double n) {
    return (vector_t){v.x*n, v.y*n, v.z*n};
}

Vector4 euler_to_q(vector_t vector, double rotation) {
    vector = normalize(vector);
    double theta = rotation / 2;
    return (Vector4){
        vector.x * sin(theta),
        vector.y * sin(theta),
        vector.z * sin(theta),
        cos(theta)
    };
}

vector_t q_to_euler(Vector4 q) {
    return normalize((vector_t){q.x, q.y, q.z});
}

Vector4 qmatrix(Vector4 q1, Vector4 q2) {
    return (Vector4){
        q1.r * q2.x + q1.x * q2.r + q1.y * q2.z - q1.z * q2.y,
        q1.r * q2.y + q1.y * q2.r + q1.z * q2.x - q1.x * q2.z,
        q1.r * q2.z + q1.z * q2.r + q1.x * q2.y - q1.y * q2.x,
        q1.r * q2.r - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z
    };
}

Vector4 qconjugate(Vector4 q) {
    return (Vector4){-q.x, -q.y, -q.z, q.r};
}

Vector4 qrotate(Vector4 q1, vector_t v) {
    Vector4 q2 = (Vector4){v.x, v.y, v.z, 0};
    return qmatrix(qmatrix(q1, q2), qconjugate(q1));
}

Vector4 qmul(Vector4 q1, Vector4 q2) {
    return qmatrix(qmatrix(q1, q2), qconjugate(q1));
}

void gyro_world_correction(double rate) {
    vector_t accel = imu_read_accel();
    double x = -accel.x / BIT_14;
    double y = -accel.y / BIT_14;
    double z = accel.z / BIT_14;
    vector_t accel_top = normalize((vector_t){-x, -y, -z});
    if (!init) {
        // world_top = (vector_t){0, 0, 1};
        // world_fw = (vector_t){0, 1, 0};
        // world_right = (vector_t){1, 0, 0};
        world_top = accel_top;
        world_fw = cross_product(world_top, (vector_t){1, 0, 0});
        world_right = cross_product(world_fw, world_top);
        init = true;
    } else {
        vector_t correction_axis = cross_product(world_top, accel_top);
        Vector4 correction = euler_to_q(correction_axis, rate);
        world_top = q_to_euler(qrotate(correction, world_top));
        world_right = q_to_euler(qrotate(correction, world_right));
        world_fw = normalize(cross_product(world_top, world_right));
    }
}

double hssnf(double t, double k, double x) {
    double a = x - (x * k);
    double b = 1 - (x * k * (1/t));
    return a / b;
}

void Gyro__report_absolute(Gyro *self) {
    gyro_world_correction(0.0001);
    vector_t gyro = imu_read_gyro();
    static double s = BIT_18 * M_PI;
    Vector4 rx = euler_to_q(world_right, -gyro.y / s);
    Vector4 ry = euler_to_q(world_fw, -gyro.z / s);
    Vector4 rz = euler_to_q(world_top, -gyro.x / s);
    static uint8_t i = 0;
    Vector4 r;
    if      (i==0) r = qmatrix(qmatrix(rx, ry), rz);
    else if (i==1) r = qmatrix(qmatrix(rz, rx), ry);
    else if (i==2) r = qmatrix(qmatrix(ry, rz), rx);
    else if (i==3) r = qmatrix(qmatrix(rx, rz), ry);
    else if (i==4) r = qmatrix(qmatrix(ry, rx), rz);
    else if (i==5) r = qmatrix(qmatrix(rz, ry), rx);
    i++;
    if (i>5) i = 0;
    world_top = q_to_euler(qrotate(r, world_top));
    world_fw = q_to_euler(qrotate(r, world_fw));
    world_right = cross_product(world_fw, world_top);

    // Combo.
    // hid_gamepad_lx(world_top.x);
    // hid_gamepad_ly(-world_top.y);
    // hid_gamepad_rx(world_fw.x);
    // hid_gamepad_ry(-world_fw.y);

    // Tilt.
    double x = -world_right.z;
    x = x > 0 ? ramp_inv(x, antideadzone) : -ramp_inv(-x, antideadzone);
    hid_gamepad_lx(x);
    // printf("\r%6.3f", x);
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
    init = false;
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
    world_top = (vector_t){0, 0, 1};
    world_fw = (vector_t){0, 1, 0};
    world_right = (vector_t){1, 0, 0};
    return gyro;
}
