// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include <pico/stdlib.h>
#include "config.h"
#include "touch.h"
#include "pin.h"
#include "helper.h"

uint8_t loglevel = 0;
uint8_t threshold_config = 0;
bool touched = false;

void touch_update_threshold() {
    config_nvm_t config;
    config_read(&config);
    float values[5] = {
        CFG_TOUCH_THRESHOLD_0,
        CFG_TOUCH_THRESHOLD_1,
        CFG_TOUCH_THRESHOLD_2,
        CFG_TOUCH_THRESHOLD_3,
        CFG_TOUCH_THRESHOLD_4
    };
    threshold_config = values[config.touch_threshold];
}

void touch_init() {
    printf("Config touch: ");
    gpio_init(PIN_TOUCH_OUT);
    gpio_set_dir(PIN_TOUCH_OUT, GPIO_OUT);
    gpio_init(PIN_TOUCH_IN);
    gpio_set_dir(PIN_TOUCH_IN, GPIO_IN);
    gpio_set_pulls(PIN_TOUCH_IN, false, false);
    touch_update_threshold();
    printf("completed\n");
}

bool touch_status() {
    // Send low.
    uint32_t time_low;
    busy_wait_us_32(CFG_TOUCH_SETTLE);
    time_low = time_us_32();
    gpio_put(PIN_TOUCH_OUT, false);
    while(gpio_get(PIN_TOUCH_IN) == true) {
        if ((time_us_32() - time_low) > CFG_TOUCH_TIMEOUT) {
            return touched;
        }
    };
    // Send high.
    busy_wait_us_32(CFG_TOUCH_SETTLE);
    time_low = time_us_32();
    gpio_put(PIN_TOUCH_OUT, true);
    while(gpio_get(PIN_TOUCH_IN) == false) {
        if ((time_us_32() - time_low) > CFG_TOUCH_TIMEOUT) {
            break;
        }
    };
    // Determine capacitance low-to-high elapsed time.
    uint32_t timing;
    timing = time_us_32() - time_low;

    // Debug.
    if (loglevel > 1) {
        static uint16_t x= 0;
        x++;
        if (!(x % 20)) printf("%i ", timing);
    }

    // Determine if the surface is considered touched and report.
    static bool touched_prev = false;
    static uint8_t repeated = 0;
    static uint8_t peak = 0;
    uint8_t threshold = threshold_config;
    if (threshold_config == 0) {
        peak = max(peak, timing);
        threshold = max(2, peak / 2);
    }
    touched = (timing >= threshold) || (timing == 0);
    if (touched != touched_prev) {
        // Only report change on repeated threshold hits.
        repeated++;
        if (repeated >= CFG_TOUCH_SMOOTH) {
            touched_prev = touched;
            return touched;
        }
    } else {
        repeated = 0;
        return touched_prev;
    }
}
