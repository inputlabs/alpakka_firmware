// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#pragma once

typedef enum LEDMode_enum {
    LED_MODE_IDLE,
    LED_MODE_STATIC,
    LED_MODE_BLINK,
    LED_MODE_CYCLE,
} LEDMode;

#define LED_BLINK_PERIOD 100  // milliseconds.
#define LED_WARNING_PERIOD 300  // milliseconds.

#define LED_UP 1
#define LED_RIGHT 2
#define LED_DOWN 4
#define LED_LEFT 8

#define LED_NONE 0b0000
#define LED_ALL 0b1111

#define LED_TRIANGLE_UP     LED_UP    + LED_LEFT + LED_RIGHT
#define LED_TRIANGLE_RIGHT  LED_RIGHT + LED_UP   + LED_DOWN
#define LED_TRIANGLE_DOWN   LED_DOWN  + LED_LEFT + LED_RIGHT
#define LED_TRIANGLE_LEFT   LED_LEFT  + LED_UP   + LED_DOWN

void led_init();
void led_set_mode(LEDMode target_mode);
void led_idle_mask(uint8_t mask);
void led_static_mask(uint8_t mask);
void led_blink_mask(uint8_t mask);
void led_cycle_step();
void led_set_warning_calibration(bool state);
void led_set_warning_gyro(bool state);
void led_ignore_warnings();
