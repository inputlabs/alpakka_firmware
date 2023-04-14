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
uint8_t peak = 0;

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
    printf("INIT: Touch\n");
    gpio_init(PIN_TOUCH_OUT);
    gpio_set_dir(PIN_TOUCH_OUT, GPIO_OUT);
    gpio_init(PIN_TOUCH_IN);
    gpio_set_dir(PIN_TOUCH_IN, GPIO_IN);
    gpio_set_pulls(PIN_TOUCH_IN, false, false);
    touch_update_threshold();
}

uint8_t touch_get_dynamic_threshold(uint8_t timing) {
    static uint8_t timing_prev = 0;
    static uint16_t ticks = 0;
    ticks++;
    // Lower threshold periodically.
    uint32_t pushdown_interval = 1200000 / (pow(peak, 4));  // Sorry for the magic numbers,
    if (!(ticks % pushdown_interval)) {                     // this is experimental.
        uint8_t peak_candidate = peak - 1;
        uint8_t peak_min = CFG_TOUCH_DYNAMIC_MIN * CFG_TOUCH_DYNAMIC_FACTOR;
        if (peak_candidate >= peak_min) {
            peak = peak_candidate;
            ticks = 0;
            if (loglevel >= 2) printf("Touch peak down %i\n", peak);
        }
    }
    // Raise threshold.
    if (timing > peak && timing_prev > peak) {
        peak = min(timing, timing_prev);
        if (loglevel >= 2) printf("Touch peak up %i\n", peak);
    }
    // Return.
    timing_prev = timing;
    return max(CFG_TOUCH_DYNAMIC_MIN, peak / CFG_TOUCH_DYNAMIC_FACTOR);
}

bool touch_status() {
    // Send low.
    uint32_t time_low;
    busy_wait_us_32(CFG_TOUCH_SETTLE);
    time_low = time_us_32();
    gpio_put(PIN_TOUCH_OUT, false);
    while(gpio_get(PIN_TOUCH_IN) == true) {
        if ((time_us_32() - time_low) > CFG_TOUCH_TIMEOUT) {
            if (loglevel >= 1) printf("Touch send low timeout\n");
            return true;
        }
    };
    // Send high.
    busy_wait_us_32(CFG_TOUCH_SETTLE);
    time_low = time_us_32();
    gpio_put(PIN_TOUCH_OUT, true);
    while(gpio_get(PIN_TOUCH_IN) == false) {
        if ((time_us_32() - time_low) > CFG_TOUCH_TIMEOUT) {
            if (loglevel >= 1) printf("Touch send high timeout\n");
            return true;
        }
    };
    // Determine capacitance low-to-high elapsed time.
    uint32_t timing;
    timing = time_us_32() - time_low;

    // Debug.
    if (loglevel >= 2) {
        static uint16_t x= 0;
        x++;
        if (!(x % 20)) printf("%i %i\n", timing, peak);
    }

    // Determine if the surface is considered touched and report.
    static bool touched = false;
    static uint8_t hits = 0;
    uint8_t threshold = threshold_config;
    if (threshold_config == 0) {
        threshold = touch_get_dynamic_threshold(timing);
    }
    bool over = timing >= threshold;
    if (over != touched) {
        // Only report change on repeated hits.
        hits++;
        if (hits >= CFG_TOUCH_SMOOTH) {
            touched = over;
            if (loglevel >= 1) printf("Touch status %i\n", touched);
        }
    } else {
        hits = 0;
    }
    return touched;
}
