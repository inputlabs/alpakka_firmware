// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#pragma once

typedef enum GyroMode_enum {
    GYRO_MODE_ALWAYS_OFF,
    GYRO_MODE_ALWAYS_ON,
    GYRO_MODE_TOUCH_OFF,
    GYRO_MODE_TOUCH_ON,
} GyroMode;

typedef struct Gyro_struct Gyro;
struct Gyro_struct {
    bool (*is_engaged) (Gyro *self);
    void (*report) (Gyro *self);
    void (*reset) (Gyro *self);
    uint8_t mode;
    uint8_t pin;
    Button engage_button;
    uint8_t actions_x[4];
    uint8_t actions_y[4];
    uint8_t actions_z[4];
};

Gyro Gyro_ (
    uint8_t mode,
    uint8_t pin,
    ...  // Actions.
);

void gyro_report();
void gyro_reset();
