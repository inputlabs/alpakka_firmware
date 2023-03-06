// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include <pico/stdlib.h>
#include "config.h"
#include "touch.h"
#include "pin.h"
#include "helper.h"

bool touched = false;
uint8_t threshold = 0;
uint8_t loglevel = 0;

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
    threshold = values[config.touch_threshold];
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

bool touch_status_static(uint8_t timing) {
    static bool touched_prev = false;
    static uint8_t repeated = 0;
    touched = (timing >= threshold) || (timing == 0);
    // Only report change on repeated matches.
    if (touched == touched_prev) {
        repeated = 0;
    } else {
        repeated++;
        if (repeated >= CFG_TOUCH_SMOOTH) {
            touched_prev = touched;
            return touched;
        }
    }
    return touched_prev;
}

bool touch_status_dynamic(uint8_t timing) {
    static uint8_t peak_up = 0;
    static uint8_t peak_down = 1;
    static uint8_t buffer[32] = {0,};
    static uint8_t buffer_index = 0;
    // Buffer.
    buffer[buffer_index] = timing;
    buffer_index++;
    if (buffer_index >= 32) buffer_index = 0;
    // If the surface is currently being reported as not touched.
    if (!touched) {
        // If the current timing is lower than the current peak
        // then in becomes the new low peak.
        if (timing < peak_down) {
            peak_down = timing;
            if (loglevel > 0) printf("down down %i %i~%i\n", timing, peak_down, peak_up);
            return false;
        }
        // If the timing value goes over the peak, it is considered touched.
        if (timing > peak_down + 1) {
            touched = true;
            if (loglevel > 0) printf("down up   %i %i~%i\n", timing, peak_down, peak_up);
            return true;
        }
    }
    // If the surface is currently being reported as touched.
    else if (touched) {
        // If the timing value goes under the dynamic threshold, it is
        // considered not touched anymore.
        peak_up = 0;
        for(uint8_t i=0; i<32; i++) {
            peak_up = max(peak_up, buffer[buffer_index]);
        }
        uint8_t dynamic_threshold = max(2, peak_up / 2);
        if (timing < dynamic_threshold) {
            touched = false;
            peak_down = timing;
            if (loglevel > 0) printf("up down   %i %i~%i\n", timing, peak_down, peak_up);
            return false;
        }
    }
    return touched;
}

bool touch_status() {
    uint32_t timing;
    uint32_t time_low;
    // Send low.
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
    timing = time_us_32() - time_low;

    // Debug.
    if (loglevel > 1) {
        static uint16_t x= 0;
        x++;
        if (!(x % 20)) printf("%i ", timing);
    }

    // Report.
    if (threshold == 0) return touch_status_dynamic(timing);
    else return touch_status_static(timing);
}
