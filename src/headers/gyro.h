// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#pragma once

typedef enum GyroMode_enum {
    GYRO_MODE_ALWAYS_OFF,
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
    void (*config_absolute_x_range) (Gyro *self, double min, double max);
    void (*config_absolute_y_range) (Gyro *self, double min, double max);
    uint8_t mode;
    uint8_t pin;
    Button engage_button;
    double absolute_x_min;
    double absolute_x_max;
    double absolute_y_min;
    double absolute_y_max;
    bool pressed_x;
    bool pressed_y;
    bool pressed_x_neg;
    bool pressed_y_neg;
    uint8_t actions_x[4];
    uint8_t actions_y[4];
    uint8_t actions_z[4];
    uint8_t actions_x_neg[4];
    uint8_t actions_y_neg[4];
    uint8_t actions_z_neg[4];
};

Gyro Gyro_ (
    GyroMode mode,
    uint8_t pin,
    ...  // Actions.
);

void gyro_update_sensitivity();
void gyro_wheel_antideadzone(int8_t increment);
