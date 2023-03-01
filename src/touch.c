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
bool status_prev = false;
uint8_t repeated = 0;
uint8_t threshold = 0;
uint8_t peak_up = 0;
uint8_t peak_down = 0;

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
    if (!status) {
        if (delta < peak_down) {
            peak_down = delta;
            // printf("down down %i %i\n", delta, peak_down);
            return false;
        }
        if (delta > peak_down + 1) {
            status = true;
            peak_down = 0;
            // printf("down up %i %i\n", delta, peak_down);
            return true;
        }
    }
    if (status) {
        if (delta > peak_up) {
            peak_up = delta;
            // printf("up up %i %i\n", delta, peak_up);
            return true;
        }
        uint8_t dynamic_threshold = max(2, peak_up / 2);
        if (delta < dynamic_threshold) {
            status = false;
            peak_up = 0;
            peak_down = delta;
            // printf("up down %i %i\n", delta, peak_up);
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
    // Measure current timing and report.
    delta = time_us_32() - sent_time;
    // printf("%i ", delta);
    if (threshold == 0) return touch_status_dynamic();
    else return touch_status_static();
}
