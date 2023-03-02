// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#pragma once
#include <stdbool.h>
#include <pico/stdlib.h>

#define SENTINEL 255
#define ACTIONS(...)  __VA_ARGS__, SENTINEL

typedef enum button_mode {
    NORMAL,
    STICKY,
    HOLD_EXCLUSIVE,
    HOLD_EXCLUSIVE_LONG,
    HOLD_OVERLAP,
    HOLD_OVERLAP_EARLY,
    CYCLE
} Button_mode;

typedef struct Button_struct Button;
struct Button_struct {
    bool (*is_pressed) (Button *self);
    void (*report) (Button *self);
    void (*reset) (Button *self);
    void (*handle_normal) (Button *self);
    void (*handle_sticky) (Button *self);
    void (*handle_hold_exclusive) (Button *self, uint16_t time);
    void (*handle_hold_overlap) (Button *self);
    void (*handle_hold_overlap_early) (Button *self);
    void (*handle_cycle) (Button *self);
    Button_mode behavior;
    uint8_t pin;
    uint8_t actions[4];
    uint8_t actions_secondary[4];
    bool state;
    bool state_secondary;
    bool virtual_press;
    bool virtual_press_secondary;
    uint64_t press_timestamp;
    uint64_t hold_timestamp;
};

Button Button_ (
    uint8_t pin,
    uint8_t behavior,
    ...  // Actions.
);
