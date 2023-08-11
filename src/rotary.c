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
#include "logging.h"

void rotary_callback(uint gpio, uint32_t events) {
    Profile* profile = profile_get_active(false);
    Rotary* rotary = &(profile->rotary);
    rotary->timestamp = time_us_32();
    rotary->increment = gpio_get(PIN_ROTARY_A) ^ gpio_get(PIN_ROTARY_B) ? -1 : 1;
    rotary->pending = true;
}

void rotary_init() {
    info("INIT: Rotary\n");
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
        uint8_t actions[8] = {0,};
        for(uint8_t r=0; r<abs(self->increment); r++) {
            for(uint8_t i=0; i<ACTIONS_LEN; i++) {
                uint8_t action = (
                    self->increment > 0
                    ? self->actions_up[i]
                    : self->actions_down[i]
                );
                hid_press(action);
                hid_release_later(action, 100);
            }
        }
        self->increment = 0;
        self->pending = false;
    }
}

void Rotary__reset(Rotary *self) {
    self->pending = false;
    self->increment = 0;
    self->timestamp = 0;
}

Rotary Rotary_ (
    void *useless,
    ...  // Actions
) {
    Rotary rotary;
    rotary.report = Rotary__report;
    rotary.reset = Rotary__reset;
    rotary.pending = false;
    rotary.increment = 0;
    rotary.timestamp = 0;
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
