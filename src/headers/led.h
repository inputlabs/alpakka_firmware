// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#pragma once

#define LED_UP 0
#define LED_RIGHT 1
#define LED_DOWN 2
#define LED_LEFT 3

#define LED_MASK_UP 1
#define LED_MASK_RIGHT 2
#define LED_MASK_DOWN 4
#define LED_MASK_LEFT 8

#define LED_MASK_TRIANGLE_UP  LED_MASK_UP + LED_MASK_LEFT + LED_MASK_RIGHT
#define LED_MASK_TRIANGLE_RIGHT  LED_MASK_RIGHT + LED_MASK_UP + LED_MASK_DOWN
#define LED_MASK_TRIANGLE_DOWN  LED_MASK_DOWN + LED_MASK_LEFT + LED_MASK_RIGHT
#define LED_MASK_TRIANGLE_LEFT  LED_MASK_LEFT + LED_MASK_UP + LED_MASK_DOWN

void led_init();
void led_set(uint8_t led, bool state);
void led_mask(uint8_t mask);
void led_cycle();
void led_cycle_step();
void led_blink_mask(uint8_t mask);
void led_blink_step();
void led_stop();
void led_shape_all_off();
void led_shape_all_on();
void led_shape_triangle_up();
void led_shape_triangle_down();
void led_shape_triangle_left();
void led_shape_triangle_right();
void led_shape_blink_triangle_up();
