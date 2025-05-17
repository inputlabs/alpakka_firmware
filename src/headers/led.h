// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#pragma once

typedef enum LEDMode_enum {
    LED_MODE_IDLE,
    LED_MODE_ENGAGE,
    LED_MODE_BLINK,
    LED_MODE_CYCLE,
} LEDMode;

#define LED_BLINK_PERIOD 100  // milliseconds.
#define LED_WARNING_PERIOD 300  // milliseconds.

#define LED_UP    0b0001
#define LED_RIGHT 0b0010
#define LED_DOWN  0b0100
#define LED_LEFT  0b1000

#define LED_NONE  0b0000
#define LED_ALL   0b1111

#define LED_TRIANGLE_UP     LED_UP    + LED_LEFT + LED_RIGHT
#define LED_TRIANGLE_RIGHT  LED_RIGHT + LED_UP   + LED_DOWN
#define LED_TRIANGLE_DOWN   LED_DOWN  + LED_LEFT + LED_RIGHT
#define LED_TRIANGLE_LEFT   LED_LEFT  + LED_UP   + LED_DOWN

void led_init();
void led_set_mode(LEDMode target_mode);
void led_idle_mask(uint8_t mask);
void led_static_mask(uint8_t mask);
void led_blink_mask(uint8_t mask);
void led_show();
void led_show_cycle_step();
void led_board_set(bool state);
