// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#pragma once

#define CFG_TOUCH_SETTLE 20  // Microseconds.
#define CFG_TOUCH_THRESHOLD 4  // Microseconds.
#define CFG_TOUCH_TIMEOUT 50  // Microseconds.
#define CFG_TOUCH_SMOOTH 2

void touch_init();
bool touch_status();
