// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#pragma once
#include <stdbool.h>
#include <pico/stdlib.h>
#include "ctrl.h"
#include "common.h"

#define ACTIONS_LEN 4

typedef uint8_t Actions[ACTIONS_LEN];

// https://inputlabs.io/alpakka/manual/dev_profiles#button_mode
typedef enum ButtonMode_enum {
    NORMAL,
    STICKY,
    HOLD_EXCLUSIVE,
    HOLD_EXCLUSIVE_LONG,
    HOLD_OVERLAP,
    HOLD_OVERLAP_LONG,
    HOLD_DOUBLE_PRESS,
} ButtonMode;

typedef struct Button_struct Button;
struct Button_struct {
    bool (*is_pressed) (Button *self);
    void (*report) (Button *self);
    void (*reset) (Button *self);
    void (*handle_normal) (Button *self);
    void (*handle_sticky) (Button *self);
    void (*handle_hold_exclusive) (Button *self, uint16_t time);
    void (*handle_hold_overlap) (Button *self,  uint16_t time);
    void (*handle_hold_double_press) (Button *self);
    uint8_t pin;
    ButtonMode mode;
    Actions actions;
    Actions actions_secondary;
    bool state;
    bool state_secondary;
    bool virtual_press;
    uint64_t press_timestamp;
    uint64_t hold_timestamp;
};

Button Button_ (
    uint8_t pin,
    ButtonMode mode,
    Actions actions,
    Actions actions_secondary
);

Button Button_from_ctrl(
    uint8_t pin,
    CtrlSection section
);
