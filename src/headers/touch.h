// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#pragma once

#define CFG_TOUCH_SETTLE 20  // Microseconds.
#define CFG_TOUCH_TIMEOUT 50  // Microseconds.
#define CFG_TOUCH_SMOOTH 2

#define CFG_TOUCH_THRESHOLD_0 2  // Microseconds.
#define CFG_TOUCH_THRESHOLD_1 3  // Microseconds.
#define CFG_TOUCH_THRESHOLD_2 5  // Microseconds.
#define CFG_TOUCH_THRESHOLD_3 8  // Microseconds.
#define CFG_TOUCH_THRESHOLD_4 13  // Microseconds.

void touch_init();
void touch_update_threshold();
bool touch_status();
