// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#pragma once

#define CFG_TOUCH_SETTLE 20  // Microseconds.
#define CFG_TOUCH_TIMEOUT 20  // Microseconds.
#define CFG_TOUCH_SMOOTH 2  // Ticks.
#define CFG_TOUCH_DYNAMIC_MIN 3 // Microseconds
#define CFG_TOUCH_DYNAMIC_FACTOR 2

#define CFG_TOUCH_THRESHOLD_0 0  // Automatic.
#define CFG_TOUCH_THRESHOLD_1 2  // Microseconds.
#define CFG_TOUCH_THRESHOLD_2 3  // Microseconds.
#define CFG_TOUCH_THRESHOLD_3 5  // Microseconds.
#define CFG_TOUCH_THRESHOLD_4 8  // Microseconds.

void touch_init();
void touch_update_threshold();
bool touch_status();
