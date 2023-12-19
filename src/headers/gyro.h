// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#pragma once

typedef enum GyroMode_enum {
    GYRO_MODE_OFF,
    GYRO_MODE_ALWAYS_ON,
    GYRO_MODE_TOUCH_OFF,
    GYRO_MODE_TOUCH_ON,
    GYRO_MODE_AXIS_ABSOLUTE,
} GyroMode;

typedef struct Gyro_struct Gyro;
struct Gyro_struct {
    bool (*is_engaged) (Gyro *self);
    void (*report) (Gyro *self);
    void (*report_incremental) (Gyro *self);
    void (*report_absolute) (Gyro *self);
    void (*reset) (Gyro *self);
    void (*config_x) (Gyro *self, double min, double max, Actions neg, Actions pos);
    void (*config_y) (Gyro *self, double min, double max, Actions neg, Actions pos);
    void (*config_z) (Gyro *self, double min, double max, Actions neg, Actions pos);
    GyroMode mode;
    uint8_t engage;
    Button engage_button;
    double absolute_x_min;
    double absolute_y_min;
    double absolute_z_min;
    double absolute_x_max;
    double absolute_y_max;
    double absolute_z_max;
    bool pressed_x_pos;
    bool pressed_y_pos;
    bool pressed_z_pos;
    bool pressed_x_neg;
    bool pressed_y_neg;
    bool pressed_z_neg;
    Actions actions_x_pos;
    Actions actions_y_pos;
    Actions actions_z_pos;
    Actions actions_x_neg;
    Actions actions_y_neg;
    Actions actions_z_neg;
};

Gyro Gyro_ (
    GyroMode mode,
    uint8_t engage
);

void gyro_update_sensitivity();
void gyro_wheel_antideadzone(int8_t increment);
