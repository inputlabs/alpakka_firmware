// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include <pico/stdlib.h>
#include <pico/multicore.h>
#include <pico/time.h>
#include <tusb.h>
#include "config.h"
#include "led.h"
#include "bus.h"
#include "profile.h"
#include "touch.h"
#include "imu.h"
#include "hid.h"
#include "uart.h"

void stdio_init() {
    stdio_uart_init();
    stdio_init_all();
}

void title() {
    printf("\n");
    printf("╔═══════════════════════════╗\n");
    printf("║ Input Labs Oy.            ║\n");
    printf("║ Alpakka - firmware 0.86.2 ║\n");
    printf("╚═══════════════════════════╝\n");
}

void main_init() {
    stdio_init();
    title();
    config_init();
    led_init();
    bus_init();
    thumbstick_init();
    touch_init();
    rotary_init();
    profile_init();
    imu_init();
    tusb_init();
}

void main_loop() {
    int16_t i = 0;
    while (true) {
        // Start timer.
        uint32_t tick_start = time_us_32();
        // Report.
        profile_report_active();
        hid_report();
        // Tick interval control.
        uint32_t tick_completed = time_us_32() - tick_start;
        uint16_t tick_interval = 1000000 / CFG_TICK_FREQUENCY;
        int32_t tick_idle = tick_interval - (int32_t)tick_completed;
        // Listen to incomming UART messages.
        if (!(i % CFG_TICK_FREQUENCY)) {
            uart_listen_char();
        }
        // Print additional timing data.
        if (CFG_LOG_LEVEL && !(i % 1000)) {
            printf("Tick comp=%li idle=%i\n", tick_completed, tick_idle);
        }
        if (tick_idle > 0) {
            sleep_us((uint32_t)tick_idle);
        } else {
            printf("+");
        }
        i++;
    }
}

int main() {
    main_init();
    main_loop();
}
