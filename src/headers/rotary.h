// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#pragma once

typedef struct Rotary_struct Rotary;

struct Rotary_struct {
    void (*report) (Rotary *self);
    void (*reset) (Rotary *self);
    void (*config_mode) (Rotary *self, uint8_t mode, ...);
    bool pending;
    int8_t increment;
    int8_t mode;
    uint32_t timestamp;
    uint8_t actions[8][2][4];
};

Rotary Rotary_ (
    void *useless,
    ...  // Actions.
);

void rotary_init();
void rotary_set_mode(uint8_t value);
