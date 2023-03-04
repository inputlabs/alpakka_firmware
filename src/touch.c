// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include <pico/stdlib.h>
#include "config.h"
#include "touch.h"
#include "pin.h"
#include "helper.h"

uint32_t sent_time;
uint32_t delta;
bool status = false;
uint8_t threshold = 0;

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

bool touch_status_static() {
    static bool status_prev = false;
    static uint8_t repeated = 0;
    status = (delta >= threshold) || (delta == 0);
    // Only report change on repeated matches.
    if (status == status_prev) {
        repeated = 0;
    } else {
        repeated++;
        if (repeated >= CFG_TOUCH_SMOOTH) {
            status_prev = status;
            return status;
        }
    }
    return status_prev;
}

bool touch_status_dynamic() {
    static uint8_t peak_up = 255;
    static uint8_t peak_down = 1;
    // If the surface is currently being reported as not touched.
    if (!status) {
        // If the current timing is lower than the current peak
        // then in becomes the new low peak.
        if (delta < peak_down) {
            peak_down = delta;
            printf("down down %i %i~%i\n", delta, peak_down, peak_down);
            return false;
        }
        // If the timing value goes over the peak, it is considered touched.
        if (delta > peak_down) {
            status = true;
            printf("down up   %i %i~%i\n", delta, peak_down, peak_down);
            return true;
        }
    }
    // If the surface is currently being reported as touched.
    if (status) {
        // If the current timing is higher than the current peak
        // then in becomes the new high peak.
        if (delta > peak_up) {
            peak_up = delta;
            printf("up up     %i %i~%i\n", delta, peak_down, peak_down);
            return true;
        }
        // If the timing value goes under the dynamic threshold, it is
        // considered not touched anymore.
        uint8_t dynamic_threshold = max(2, peak_up / 2);
        if (delta < dynamic_threshold) {
            status = false;
            peak_up = 0;
            peak_down = delta;
            printf("up down   %i %i~%i\n", delta, peak_down, peak_down);
            return false;
        }
    }
    return status;
}

bool touch_status() {
    // Send low.
    busy_wait_us_32(CFG_TOUCH_SETTLE);
    sent_time = time_us_32();
    gpio_put(PIN_TOUCH_OUT, false);
    while(gpio_get(PIN_TOUCH_IN) == true) {
        if ((time_us_32() - sent_time) > CFG_TOUCH_TIMEOUT) {
            printf("B0\n");
            break;
        }
    };
    // Send high.
    busy_wait_us_32(CFG_TOUCH_SETTLE);
    sent_time = time_us_32();
    gpio_put(PIN_TOUCH_OUT, true);
    while(gpio_get(PIN_TOUCH_IN) == false) {
        if ((time_us_32() - sent_time) > CFG_TOUCH_TIMEOUT) {
            printf("B1\n");
            break;
        }
    };
    // Measure current timing and report.
    delta = time_us_32() - sent_time;
    static uint16_t x= 0;
    x++;
    if (!(x % 20)) printf("%i ", delta);
    if (threshold == 0) return touch_status_dynamic();
    else return touch_status_static();
}
