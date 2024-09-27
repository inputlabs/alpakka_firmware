// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#pragma once

// The maximum elapsed time before the measurement is assumed infinite.
// Also the time limit to perform multiple measurements.
#define TOUCH_TIMEOUT 100  // Microseconds.

// How is the measurement performed.
// settled is 0 -> measurement 0 to 1 -> How long it takes to charge up the capacitor.
// settled is 1 -> measurement 1 to 0 -> How long it takes to discharge the capacitor.
#define TOUCH_SETTLED_STATE 1

// The starting baseline threshold value when using dynamic.
#define TOUCH_AUTO_START_GEN0 2  // Microseconds.
#define TOUCH_AUTO_START_GEN1 10 // Microseconds.

// Dynamic threshold algorithm tuning.
#define TOUCH_AUTO_RATIO 1.5  // Should be configurable?
#define TOUCH_AUTO_SMOOTH  (CFG_TICK_FREQUENCY)  // 1 second.

// Debounce.
#define TOUCH_DEBOUNCE 100  // Milliseconds.

// Debug.
#define TOUCH_DEBUG_FREQ 1000  // Milliseconds.

void touch_init();
void touch_update_threshold();
bool touch_status();
