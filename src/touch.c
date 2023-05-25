// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include <math.h>
#include <pico/stdlib.h>
#include "config.h"
#include "touch.h"
#include "pin.h"
#include "helper.h"

uint8_t loglevel = 0;
uint8_t sens_from_config = 0;
uint8_t dynamic_min = 0;
uint8_t timeout = 0;
float threshold = 0;

void touch_update_threshold() {
    config_nvm_t config;
    config_read(&config);
    uint8_t values_gen0[5] = {
        CFG_GEN0_TOUCH_SENS_0,
        CFG_GEN0_TOUCH_SENS_1,
        CFG_GEN0_TOUCH_SENS_2,
        CFG_GEN0_TOUCH_SENS_3,
        CFG_GEN0_TOUCH_SENS_4
    };
    uint8_t values_gen1[5] = {
        CFG_GEN1_TOUCH_SENS_0,
        CFG_GEN1_TOUCH_SENS_1,
        CFG_GEN1_TOUCH_SENS_2,
        CFG_GEN1_TOUCH_SENS_3,
        CFG_GEN1_TOUCH_SENS_4
    };
    // See https://github.com/inputlabs/alpakka_pcb/blob/main/generations.md
    if (config_get_pcb_gen() == 0) {
         // PCB gen 0.
        sens_from_config = values_gen0[config.touch_threshold];
        timeout = CFG_GEN0_TOUCH_TIMEOUT;
        dynamic_min = CFG_GEN0_TOUCH_DYNAMIC_MIN;
    } else {
        // PCB gen 1+.
        sens_from_config = values_gen1[config.touch_threshold];
        timeout = CFG_GEN1_TOUCH_TIMEOUT;
        dynamic_min = CFG_GEN1_TOUCH_DYNAMIC_MIN;
    }
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

uint32_t touch_get_elapsed() {
    uint32_t time_low;
    time_low = time_us_32();
    gpio_put(PIN_TOUCH_OUT, true);
    bool timedout = false;
    while(gpio_get(PIN_TOUCH_IN) == false) {
        if ((time_us_32() - time_low) > timeout) {
            timedout = true;
            break;
        }
    };
    uint32_t elapsed = timedout ? 0 : time_us_32() - time_low;
    if (loglevel >= 1 && timedout) printf("T");
    gpio_put(PIN_TOUCH_OUT, false); // Send low (so is ready for next cycle).
    return elapsed;
}

float touch_get_dynamic_threshold(uint8_t elapsed) {
    static float peak = 0;
    static uint8_t elapsed_prev = 0;
    static uint16_t ticks = 0;
    ticks++;
    // Push down:
    // A periodic but slow decrease of the peak, to avoid ever-growing peaks
    // in long gaming sessions. The hyperbolic function makes it so the
    // decrease is faster the more it deviates from the minimum.
    if (!(ticks % CFG_TOUCH_DYNAMIC_PUSHDOWN_FREQ)) {
        float x = dynamic_min / peak;
        float factor = tanhf(x * CFG_TOUCH_DYNAMIC_PUSHDOWN_HYPERBOLIC);
        peak = max(dynamic_min, peak * factor);
    }
    // Push up:
    // Raise the peak as soon as the current peak has been exceeded twice.
    // (Twice to avoid fluke peaks).
    if (elapsed > peak && elapsed_prev > peak) {
        peak = min(elapsed, elapsed_prev);
    }
    // Return.
    elapsed_prev = elapsed;
    return max(dynamic_min, peak * CFG_TOUCH_DYNAMIC_PEAK_RATIO);
}

bool touch_status() {
    uint32_t elapsed = touch_get_elapsed();
    // Determine threshold.
    if (elapsed != 0) {
        threshold = (
            sens_from_config > 0 ?
            sens_from_config :
            touch_get_dynamic_threshold(elapsed)
        );
    } else {
        elapsed = threshold + 1;  // Using threshold from previous cycle.
    }
    // Debug.
    if (loglevel >= 2) {
        static uint16_t x = 0;
        x++;
        if (!(x % DEBUG_TOUCH_ELAPSED_FREQ)) {
            printf("%i %.2f\n", elapsed, threshold);
        }
    }
    // Determine if the surface is considered touched and report.
    static bool touched = false;
    static uint8_t hits = 0;
    bool over = elapsed >= threshold;
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
