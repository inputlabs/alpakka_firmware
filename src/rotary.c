// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <pico/time.h>
#include <hardware/gpio.h>
#include "config.h"
#include "pin.h"
#include "profile.h"
#include "button.h"
#include "rotary.h"
#include "hid.h"

void rotary_set_mode(uint8_t value) {
    Profile* profile = profile_get_active(false);
    Rotary* rotary = &(profile->rotary);
    rotary->mode = value;
}

void rotary_callback(uint gpio, uint32_t events) {
    Profile* profile = profile_get_active(false);
    Rotary* rotary = &(profile->rotary);
    rotary->timestamp = time_us_32();
    rotary->increment = gpio_get(PIN_ROTARY_A) ^ gpio_get(PIN_ROTARY_B) ? -1 : 1;
    rotary->pending = true;
}

void rotary_init() {
    printf("INIT: Rotary\n");
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
        self->pending &&
        (time_us_32() > (self->timestamp + CFG_MOUSE_WHEEL_DEBOUNCE))
    ) {
        for(uint8_t rotated=0; rotated<abs(self->increment); rotated++) {
            uint8_t *actions = (
                self->increment > 0 ?
                self->actions[self->mode][0] :
                self->actions[self->mode][1]
            );
            hid_press_multiple(actions);
            hid_release_multiple_later(actions, 10);
        }
        self->increment = 0;
        self->pending = false;
    }
}

void Rotary__reset(Rotary *self) {
    self->pending = false;
    self->increment = 0;
    self->timestamp = 0;
    // self->mode = 0;
}

void Rotary__config_mode(Rotary *self, uint8_t mode, ...) {
    va_list va;
    va_start(va, 0);
    for(uint8_t dir=0; dir<2; dir++) {
        bool valid = true;
        for(uint8_t action=0; action<4; action++) {
            uint8_t value = valid ? va_arg(va, int) : KEY_NONE;
            if (value == SENTINEL) valid = false;
            self->actions[mode][dir][action] = value;
        }
    }
    va_end(va);
}

Rotary Rotary_ (
    void *useless,
    ...  // Actions
) {
    Rotary rotary;
    rotary.report = Rotary__report;
    rotary.reset = Rotary__reset;
    rotary.config_mode = Rotary__config_mode;
    rotary.pending = false;
    rotary.mode = 0;
    rotary.increment = 0;
    rotary.timestamp = 0;
    va_list va;
    va_start(va, 0);
    for(uint8_t dir=0; dir<2; dir++) {
        bool valid = true;
        for(uint8_t action=0; action<4; action++) {
            uint8_t value = valid ? va_arg(va, int) : KEY_NONE;
            if (value == SENTINEL) valid = false;
            rotary.actions[0][dir][action] = value;
        }
    }
    va_end(va);
    return rotary;
}
