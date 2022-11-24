// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include <stdarg.h>
#include <pico/time.h>
#include <hardware/gpio.h>
#include "config.h"
#include "pin.h"
#include "button.h"
#include "rotary.h"
#include "hid.h"

bool rotary_pending = false;
uint32_t rotary_ts = 0;
int32_t rotary_value = 0;
int8_t rotary_increment = 0;

void rotary_callback(uint gpio, uint32_t events) {
    rotary_ts = time_us_32();
    rotary_increment = gpio_get(PIN_ROTARY_A) ^ gpio_get(PIN_ROTARY_B) ? -1 : 1;
    rotary_pending = true;
}

void rotary_init() {
    printf("Config rotary\n");
    gpio_init(PIN_ROTARY_A);
    gpio_init(PIN_ROTARY_B);
    gpio_set_dir(PIN_ROTARY_A, GPIO_IN);
    gpio_set_dir(PIN_ROTARY_B, GPIO_IN);
    gpio_pull_up(PIN_ROTARY_A);
    gpio_pull_up(PIN_ROTARY_B);
    gpio_set_irq_enabled_with_callback(
        PIN_ROTARY_A,
        0b1100,
        true,
        rotary_callback
    );
}

void Rotary__report(Rotary *self) {
    if (
        rotary_pending &&
        (time_us_32() > (rotary_ts + CFG_MOUSE_WHEEL_DEBOUNCE))
    ) {
        if (rotary_increment > 0) {
            for(uint8_t i=0; i<4; i++) {
                uint8_t action = self->actions_up[i];
                if (action == MOUSE_SCROLL_UP) hid_mouse_wheel(rotary_increment);
                if (action == MOUSE_SCROLL_DOWN) hid_mouse_wheel(-rotary_increment);
            }
            hid_press_multiple(self->actions_up);
            add_alarm_in_ms(
                100,
                (alarm_callback_t)hid_release_multiple_delayed,
                self->actions_up,
                true
            );
        }
        if (rotary_increment < 0) {
            for(uint8_t i=0; i<4; i++) {
                uint8_t action = self->actions_down[i];
                if (action == MOUSE_SCROLL_UP) hid_mouse_wheel(-rotary_increment);
                if (action == MOUSE_SCROLL_DOWN) hid_mouse_wheel(rotary_increment);
            }
            hid_press_multiple(self->actions_down);
            add_alarm_in_ms(
                100,
                (alarm_callback_t)hid_release_multiple_delayed,
                self->actions_down,
                true
            );
        }
        rotary_increment = 0;
        rotary_pending = false;
    }
}

void Rotary__reset(Rotary *self) {
}

Rotary Rotary_ (
    uint8_t useless,
    ...  // Actions
) {
    Rotary rotary;
    rotary.report = Rotary__report;
    rotary.reset = Rotary__reset;
    rotary.actions_up[0] = 0;
    rotary.actions_up[1] = 0;
    rotary.actions_up[2] = 0;
    rotary.actions_up[3] = 0;
    rotary.actions_down[0] = 0;
    rotary.actions_down[1] = 0;
    rotary.actions_down[2] = 0;
    rotary.actions_down[3] = 0;
    va_list va;
    va_start(va, 0);
    for(uint8_t i=0; true; i++) {
        uint8_t value = va_arg(va, int);
        if (value == SENTINEL) break;
        rotary.actions_up[i] = value;
    }
    for(uint8_t i=0; true; i++) {
        uint8_t value = va_arg(va, int);
        if (value == SENTINEL) break;
        rotary.actions_down[i] = value;
    }
    va_end(va);
    return rotary;
}
