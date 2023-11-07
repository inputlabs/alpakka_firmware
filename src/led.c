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

repeating_timer_t timer;
uint8_t cycle_position;
uint8_t blink_mask;
bool blink_state = false;

void led_set(uint8_t led, bool state) {
    uint8_t pins[] = {PIN_LED_UP, PIN_LED_RIGHT, PIN_LED_DOWN, PIN_LED_LEFT};
    pwm_set_gpio_level(pins[led], state ? (255 * CFG_LED_BRIGHTNESS) : 1);
}

void led_shape_all_off_stop(bool stop) {
    if (stop) led_stop();
    led_set(0, false);
    led_set(1, false);
    led_set(2, false);
    led_set(3, false);
}

void led_shape_all_off() {
    led_shape_all_off_stop(true);
}

void led_shape_all_on() {
    led_stop();
    led_set(0, true);
    led_set(1, true);
    led_set(2, true);
    led_set(3, true);
}

void led_mask(uint8_t mask) {
    if (!(0b0001 & mask)) led_set(0, false);
    if (!(0b0010 & mask)) led_set(1, false);
    if (!(0b0100 & mask)) led_set(2, false);
    if (!(0b1000 & mask)) led_set(3, false);
    if (0b0001 & mask) led_set(0, true);
    if (0b0010 & mask) led_set(1, true);
    if (0b0100 & mask) led_set(2, true);
    if (0b1000 & mask) led_set(3, true);
}

void led_cycle_step() {
    if(cycle_position > 3) cycle_position = 0;
    led_shape_all_off_stop(false);
    led_set(cycle_position, true);
    cycle_position += 1;
}

void led_cycle() {
    cycle_position = 0;
    led_stop();
    add_repeating_timer_ms(
        100,
        (repeating_timer_callback_t)led_cycle_step,
        NULL,
        &timer
    );
}

void led_blink_step() {
    if (LED_MASK_UP & blink_mask) led_set(LED_UP, blink_state);
    if (LED_MASK_RIGHT & blink_mask) led_set(LED_RIGHT, blink_state);
    if (LED_MASK_DOWN & blink_mask) led_set(LED_DOWN, blink_state);
    if (LED_MASK_LEFT & blink_mask) led_set(LED_LEFT, blink_state);
    blink_state = !blink_state;
}

void led_blink_mask(uint8_t mask) {
    led_stop();
    if (LED_MASK_UP & mask) led_set(LED_UP, true);
    if (LED_MASK_RIGHT & mask) led_set(LED_RIGHT, true);
    if (LED_MASK_DOWN & mask) led_set(LED_DOWN, true);
    if (LED_MASK_LEFT & mask) led_set(LED_LEFT, true);
    blink_state = false;
    blink_mask = mask;
    add_repeating_timer_ms(
        100,
        (repeating_timer_callback_t)led_blink_step,
        NULL,
        &timer
    );
}

void led_stop() {
    cancel_repeating_timer(&timer);
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
    led_blink_mask(0b1111);
}
