// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

/*
LED modes (and masks) define how LEDs are going to be lit.

Idle mode is a static pattern shown when the user is not interacting with the
controller, the pattern is defined my the idle mask. Alternatively if there are
pending warnings, a dynamic pattern will be shown communicating the user that
there are unresolved issues.

Engage mode is a static pattern shown when the user is actively interacting
with the controller, the pattern is defined by the engage mask.

Blink mode is a blinking pattern shown to communicate even more information
when the user is interacting with the controller, the blinking pattern is
defined both by the blink mask and the engage mask, the engage mask acting as
a kind of background pattern (see the implementation for details).

Cycle mode is a hardcoded dynamic pattern (a series of static patterns) which
is not using any mask, but just rotating a single led clockwise.

The intended use of the internal API is as follows:
- Set one (or more) masks for the mode that is intended to be used.
- Set the LED mode.
- Perform related task (eg: change the profile, do calibration, tune settings).
- Optionally set set back the LED mode to idle, or whatever was the previous mode.
*/

#include <stdio.h>
#include <pico/stdlib.h>
#include <pico/time.h>
#include <hardware/pwm.h>
#include "led.h"
#include "pin.h"
#include "config.h"
#include "common.h"
#include "logging.h"

LEDMode led_mode = LED_MODE_IDLE;

// Masks.
uint8_t idle_mask = 0;
uint8_t engage_mask = 0;
uint8_t blink_mask = 0;

// Cycling.
repeating_timer_t led_timer;
bool blink_state = false;
uint8_t cycle_position = 0;


void led_set(uint8_t pin, bool state) {
    uint8_t slice_num = pwm_gpio_to_slice_num(pin);
    pwm_clear_irq(slice_num);
    uint8_t brightness = state ? (255 * CFG_LED_BRIGHTNESS) : 0;
    pwm_set_gpio_level(pin, brightness);
}

void led_show_idle() {
    led_set(PIN_LED_UP,    LED_UP    & idle_mask);
    led_set(PIN_LED_RIGHT, LED_RIGHT & idle_mask);
    led_set(PIN_LED_DOWN,  LED_DOWN  & idle_mask);
    led_set(PIN_LED_LEFT,  LED_LEFT  & idle_mask);
}

void led_show_engage() {
    led_set(PIN_LED_UP,    LED_UP    & engage_mask);
    led_set(PIN_LED_RIGHT, LED_RIGHT & engage_mask);
    led_set(PIN_LED_DOWN,  LED_DOWN  & engage_mask);
    led_set(PIN_LED_LEFT,  LED_LEFT  & engage_mask);
}

void led_show_blink_step() {
    bool bg_up =    LED_UP    & engage_mask;
    bool bg_right = LED_RIGHT & engage_mask;
    bool bg_down =  LED_DOWN  & engage_mask;
    bool bg_left =  LED_LEFT  & engage_mask;
    led_set(PIN_LED_UP,    (LED_UP    & blink_mask) ? blink_state : bg_up);
    led_set(PIN_LED_RIGHT, (LED_RIGHT & blink_mask) ? blink_state : bg_right);
    led_set(PIN_LED_DOWN,  (LED_DOWN  & blink_mask) ? blink_state : bg_down);
    led_set(PIN_LED_LEFT,  (LED_LEFT  & blink_mask) ? blink_state : bg_left);
    blink_state = !blink_state;
}

void led_show_blink() {
    blink_state = !(engage_mask == 0b1111);  // Immediate feedback.
    add_repeating_timer_ms(
        LED_BLINK_PERIOD,
        (repeating_timer_callback_t)led_show_blink_step,
        NULL,
        &led_timer
    );
}

void led_show_cycle_step() {
    led_set(PIN_LED_UP,    cycle_position == 0);
    led_set(PIN_LED_RIGHT, cycle_position == 1);
    led_set(PIN_LED_DOWN,  cycle_position == 2);
    led_set(PIN_LED_LEFT,  cycle_position == 3);
    cycle_position += 1;
    if(cycle_position > 3) cycle_position = 0;
}

void led_show_cycle() {
    cycle_position = 0;
    add_repeating_timer_ms(
        LED_BLINK_PERIOD,
        (repeating_timer_callback_t)led_show_cycle_step,
        NULL,
        &led_timer
    );
}

void led_show_warning_step() {
    uint8_t frame_0 = LED_LEFT + LED_RIGHT;
    uint8_t frame_1 = LED_UP + LED_DOWN;
    uint8_t mask = blink_state ? frame_0 : frame_1;
    led_static_mask(mask);
    led_show_engage();
    blink_state = !blink_state;
}

void led_show_warning() {
    blink_state = false;
    add_repeating_timer_ms(
        LED_WARNING_PERIOD,
        (repeating_timer_callback_t)led_show_warning_step,
        NULL,
        &led_timer
    );
}

void led_stop() {
    cancel_repeating_timer(&led_timer);
}

void led_idle_mask(uint8_t mask) {
    idle_mask = mask;
}

void led_static_mask(uint8_t mask) {
    engage_mask = mask;
}

void led_blink_mask(uint8_t mask) {
    blink_mask = mask;
}

void led_show() {
    led_stop();
    if (led_mode == LED_MODE_IDLE) {
        if (config_problems_are_pending()) led_show_warning();
        else led_show_idle();
    }
    if (led_mode == LED_MODE_ENGAGE) led_show_engage();
    if (led_mode == LED_MODE_BLINK) led_show_blink();
    if (led_mode == LED_MODE_CYCLE) led_show_cycle();
}

void led_set_mode(LEDMode mode) {
    led_mode = mode;
    led_show();
}

void led_init_each(uint8_t pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM);
    uint8_t slice_num = pwm_gpio_to_slice_num(pin);
    pwm_set_wrap(slice_num, 255);
    pwm_set_enabled(slice_num, true);
}

void led_init() {
    // Pico LED.
    gpio_init(PIN_LED_PICO);
    gpio_set_dir(PIN_LED_PICO, GPIO_OUT);
    gpio_put(PIN_LED_PICO, true);
    // Front LEDs.
    led_init_each(PIN_LED_UP);
    led_init_each(PIN_LED_RIGHT);
    led_init_each(PIN_LED_DOWN);
    led_init_each(PIN_LED_LEFT);
    // Blink all LEDs until something else happens.
    led_static_mask(LED_NONE);
    led_blink_mask(LED_ALL);
    led_set_mode(LED_MODE_BLINK);
}
