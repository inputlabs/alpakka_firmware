// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#pragma once
#include "button.h"

#define ANALOG_FACTOR 32767
#define TRIGGER_FACTOR 255
#define ANGLE_CUT_1 22.5
#define ANGLE_CUT_2 67.5
#define ANGLE_CUT_3 112.5
#define ANGLE_CUT_4 157.5

typedef enum thumbstick_mode {
    THUMBSTICK_MODE_4DIR,
    THUMBSTICK_MODE_8DIR,
    THUMBSTICK_MODE_KEYBOARD
} Thumbstick_mode;

typedef enum thumbstick_8dir {
    DIR8_CENTER,
    DIR8_UP,
    DIR8_UP_RIGHT,
    DIR8_RIGHT,
    DIR8_DOWN_RIGHT,
    DIR8_DOWN,
    DIR8_DOWN_LEFT,
    DIR8_LEFT,
    DIR8_UP_LEFT
} Thumbstick_8dir;

typedef struct thumbstick_position {
    float x;
    float y;
    float angle;
    float radius;
} Thumbstick_position;

typedef struct Thumbstick_struct Thumbstick;
struct Thumbstick_struct {
    void (*report) (Thumbstick *self);
    void (*report_4dir) (Thumbstick *self, Thumbstick_position position);
    void (*report_keyboard) (Thumbstick *self, Thumbstick_position position);
    void (*reset) (Thumbstick *self);
    Thumbstick_mode mode;
    Button left;
    Button right;
    Button up;
    Button down;
    Button up_left;
    Button up_right;
    Button down_left;
    Button down_right;
    Button push;
    Button inner;
    Button outer;
};

Thumbstick Thumbstick_ (
    Thumbstick_mode mode,
    Button left,
    Button right,
    Button up,
    Button down,
    Button up_left,
    Button up_right,
    Button down_left,
    Button down_right,
    Button push,
    Button inner,
    Button outer
);

void thumbstick_init();
void thumbstick_report();
void thumbstick_calibrate();
void thumbstick_update_deadzone();
