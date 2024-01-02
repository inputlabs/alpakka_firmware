// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

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
uint8_t static_mask = 0;
uint8_t blink_mask = 0;

// Cycling.
repeating_timer_t led_timer;
bool blink_state = false;
uint8_t cycle_position = 0;

// Warnings.
bool warning_calibration = false;
bool warning_gyro = false;


void led_set(uint8_t pin, bool state) {
    uint8_t slice_num = pwm_gpio_to_slice_num(pin);
    pwm_clear_irq(slice_num);
    uint8_t brightness = state ? (255 * CFG_LED_BRIGHTNESS) : 0;
    pwm_set_gpio_level(pin, brightness);
}

void led_idle() {
    led_set(PIN_LED_UP,    LED_UP    & idle_mask);
    led_set(PIN_LED_RIGHT, LED_RIGHT & idle_mask);
    led_set(PIN_LED_DOWN,  LED_DOWN  & idle_mask);
    led_set(PIN_LED_LEFT,  LED_LEFT  & idle_mask);
}

void led_static() {
    led_set(PIN_LED_UP,    LED_UP    & static_mask);
    led_set(PIN_LED_RIGHT, LED_RIGHT & static_mask);
    led_set(PIN_LED_DOWN,  LED_DOWN  & static_mask);
    led_set(PIN_LED_LEFT,  LED_LEFT  & static_mask);
}

void led_blink_step() {
    bool bg_up =    LED_UP    & static_mask;
    bool bg_right = LED_RIGHT & static_mask;
    bool bg_down =  LED_DOWN  & static_mask;
    bool bg_left =  LED_LEFT  & static_mask;
    led_set(PIN_LED_UP,    (LED_UP    & blink_mask) ? blink_state : bg_up);
    led_set(PIN_LED_RIGHT, (LED_RIGHT & blink_mask) ? blink_state : bg_right);
    led_set(PIN_LED_DOWN,  (LED_DOWN  & blink_mask) ? blink_state : bg_down);
    led_set(PIN_LED_LEFT,  (LED_LEFT  & blink_mask) ? blink_state : bg_left);

    blink_state = !blink_state;
}

void led_blink() {
    blink_state = !(static_mask == 0b1111);  // Immediate feedback.
    add_repeating_timer_ms(
        LED_BLINK_PERIOD,
        (repeating_timer_callback_t)led_blink_step,
        NULL,
        &led_timer
    );
}

void led_cycle_step() {
    if(cycle_position > 3) cycle_position = 0;
    led_static(LED_NONE);
    uint8_t pins[] = {PIN_LED_UP, PIN_LED_RIGHT, PIN_LED_DOWN, PIN_LED_LEFT};
    led_set(pins[cycle_position], true);
    cycle_position += 1;
}

void led_cycle() {
    cycle_position = 0;
    add_repeating_timer_ms(
        LED_BLINK_PERIOD,
        (repeating_timer_callback_t)led_cycle_step,
        NULL,
        &led_timer
    );
}

void led_warning_step() {
    uint8_t mask = (
        blink_state ?
        (LED_LEFT + LED_RIGHT) :
        (LED_UP + LED_DOWN)
    );
    led_static_mask(mask);
    led_static();
    blink_state = !blink_state;
}

void led_warning() {
    blink_state = false;
    add_repeating_timer_ms(
        LED_WARNING_PERIOD,
        (repeating_timer_callback_t)led_warning_step,
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
    static_mask = mask;
}

void led_blink_mask(uint8_t mask) {
    blink_mask = mask;
}

bool led_warnings_are_pending() {
    return warning_calibration || warning_gyro;
}

void led_execute() {
    led_stop();
    if (led_mode == LED_MODE_IDLE) {
        if (led_warnings_are_pending()) led_warning();
        else led_idle();
    }
    if (led_mode == LED_MODE_STATIC) led_static();
    if (led_mode == LED_MODE_BLINK) led_blink();
    if (led_mode == LED_MODE_CYCLE) led_cycle();
}

void led_set_mode(LEDMode mode) {
    led_mode = mode;
    led_execute();
}

void led_set_warning_calibration(bool state) {
    warning_calibration = state;
    led_execute();
}

void led_set_warning_gyro(bool state) {
    warning_gyro = state;
    led_execute();
}

void led_ignore_warnings() {
    warn("User requested to ignore LED warnings\n");
    warning_calibration = false;
    warning_gyro = false;
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
