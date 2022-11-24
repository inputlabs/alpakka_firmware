// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#pragma once

typedef struct Rotary_struct Rotary;

struct Rotary_struct {
    void (*report) (Rotary *self);
    void (*reset) (Rotary *self);
    uint8_t actions_up[4];
    uint8_t actions_down[4];
};

Rotary Rotary_ (
    uint8_t useless,
    ...  // Actions.
);

void rotary_init();
