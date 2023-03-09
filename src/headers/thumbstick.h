// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#pragma once
#include "button.h"

#define ANALOG_FACTOR 32767
#define TRIGGER_FACTOR 255
#define DEADZONE_FROM_CONFIG 0

typedef struct thumbstick_position {
    float x;
    float y;
    float angle;
    float radius;
} Thumbstick_position;

typedef struct Thumbstick_struct Thumbstick;

struct Thumbstick_struct {
    void (*report) (Thumbstick *self);
    void (*report_4dir) (Thumbstick *self, Thumbstick_position pos, float deadzone);
    void (*reset) (Thumbstick *self);
    float deadzone;
    float overlap;
    Button left;
    Button right;
    Button up;
    Button down;
    Button push;
    Button inner;
    Button outer;
};

Thumbstick Thumbstick_ (
    float deadzone,
    float overlap,
    Button up,
    Button left,
    Button right,
    Button down,
    Button push,
    Button inner,
    Button outer
);

void thumbstick_init();
void thumbstick_report();
void thumbstick_calibrate();
void thumbstick_update_deadzone();
