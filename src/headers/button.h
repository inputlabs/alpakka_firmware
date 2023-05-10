// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#pragma once
#include <stdbool.h>
#include <pico/stdlib.h>

// Type of button behavior.
#define NORMAL 1
#define STICKY 2
#define HOLD_EXCLUSIVE 3
#define HOLD_EXCLUSIVE_LONG 4
#define HOLD_OVERLAP 5
#define HOLD_DOUBLE_PRESS 6

#define SENTINEL 255
#define MACROS_LEN 16
#define ACTIONS_LEN 4
#define ACTIONS(...)  __VA_ARGS__, SENTINEL

typedef struct Button_struct Button;

struct Button_struct {
    bool (*is_pressed) (Button *self);
    void (*report) (Button *self);
    void (*reset) (Button *self);
    void (*handle_normal) (Button *self);
    void (*handle_sticky) (Button *self);
    void (*handle_hold_exclusive) (Button *self, uint16_t time);
    void (*handle_hold_overlap) (Button *self);
    void (*handle_hold_double_press) (Button *self);
    uint8_t behavior;
    uint8_t pin;
    uint8_t actions[MACROS_LEN];
    uint8_t actions_secondary[MACROS_LEN];
    bool state;
    bool state_secondary;
    bool virtual_press;
    uint64_t press_timestamp;
    uint64_t hold_timestamp;
};

Button Button_ (
    uint8_t pin,
    uint8_t behavior,
    ...  // Actions.
);
