// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include <pico/stdlib.h>
#include "touch.h"
#include "pin.h"

uint32_t sent_time;
uint32_t delta;
bool current = false;
bool reported = false;
int8_t repeated = 0;

void touch_init() {
    printf("Config touch: ");
    gpio_init(PIN_TOUCH_OUT);
    gpio_set_dir(PIN_TOUCH_OUT, GPIO_OUT);
    gpio_init(PIN_TOUCH_IN);
    gpio_set_dir(PIN_TOUCH_IN, GPIO_IN);
    gpio_set_pulls(PIN_TOUCH_IN, false, false);
    printf("completed\n");
}

bool touch_status() {
    // Send low.
    busy_wait_us_32(CFG_TOUCH_SETTLE);
    sent_time = time_us_32();
    gpio_put(PIN_TOUCH_OUT, false);
    while(gpio_get(PIN_TOUCH_IN) == true) {
        if ((time_us_32() - sent_time) > CFG_TOUCH_TIMEOUT) {
            break;
        }
    };
    // Send high.
    busy_wait_us_32(CFG_TOUCH_SETTLE);
    sent_time = time_us_32();
    gpio_put(PIN_TOUCH_OUT, true);
    while(gpio_get(PIN_TOUCH_IN) == false) {
        if ((time_us_32() - sent_time) > CFG_TOUCH_TIMEOUT) {
            break;
        }
    };
    // Determine if surface is being touched.
    delta = time_us_32() - sent_time;
    current = (delta > CFG_TOUCH_THRESHOLD) || (delta == 0);
    // Only report change on repeated matches.
    if (current == reported) {
        repeated = 0;
    } else {
        repeated++;
        if (repeated >= CFG_TOUCH_SMOOTH) {
            reported = current;
        }
    }
    // printf("%i-", delta);
    // if (reported) printf("%i-", delta);
    return reported;
}
