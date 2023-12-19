// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#pragma once

typedef enum RotaryDir_enum {
    ROTARY_UP,
    ROTARY_DOWN,
} RotaryDir;

typedef struct Rotary_struct Rotary;
struct Rotary_struct {
    void (*report) (Rotary *self);
    void (*reset) (Rotary *self);
    void (*config_mode) (Rotary *self, uint8_t mode, Actions actions_up, Actions actions_down);
    bool pending;
    int8_t increment;
    int8_t mode;
    uint32_t timestamp;
    // Memory allocation for 5 modes, 2 directions per mode, 4 actions per
    // direction.
    uint8_t actions[5][2][4];
};

Rotary Rotary_ ();

void rotary_init();
void rotary_set_mode(uint8_t value);
