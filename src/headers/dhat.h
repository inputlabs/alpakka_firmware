// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#pragma once

typedef struct Dhat_struct Dhat;

struct Dhat_struct {
    void (*report) (Dhat *self);
    void (*reset) (Dhat *self);
    uint64_t timestamp;
    // Real buttons.
    Button left;
    Button right;
    Button up;
    Button down;
    Button push;
    // Vitual buttons.
    Button up_left;
    Button up_center;
    Button up_right;
    Button mid_left;
    Button mid_center;
    Button mid_right;
    Button down_left;
    Button down_center;
    Button down_right;
};

Dhat Dhat_ (
    Button up_left,
    Button up_center,
    Button up_right,
    Button mid_left,
    Button mid_center,
    Button mid_right,
    Button down_left,
    Button down_center,
    Button down_right
);

void dhat_report();
void dhat_reset();
