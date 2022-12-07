// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#pragma once

#define GYRO_MODE_ALWAYS_OFF 0
#define GYRO_MODE_ALWAYS_ON 1
#define GYRO_MODE_TOUCH_OFF 2
#define GYRO_MODE_TOUCH_ON 3

typedef struct Gyro_struct Gyro;

struct Gyro_struct {
    void (*report) (Gyro *self);
    void (*reset) (Gyro *self);
    uint8_t mode;
    uint8_t actions_x[4];
    uint8_t actions_y[4];
    uint8_t actions_z[4];
};

Gyro Gyro_ (
    uint8_t mode,
    ...  // Actions.
);

void gyro_report();
void gyro_reset();
