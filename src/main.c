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
#include "logging.h"

#if __has_include("version.h")
    #include "version.h"
#else
    #define VERSION "undefined"
#endif

void title() {
    info("\n");
    info("╔====================╗\n");
    info("║ Input Labs Oy.     ║\n");
    info("║ Alpakka controller ║\n");
    info("╚====================╝\n");
    info("Firmware version: %s\n", VERSION);
}

void main_init() {
    stdio_uart_init();
    stdio_init_all();
    tusb_init();
    title();
    config_init();
    bus_init();
    hid_init();
    led_init();
    thumbstick_init();
    touch_init();
    rotary_init();
    profile_init();
    imu_init();
}

void main_loop() {
    int16_t i = 0;
    webusb_set_onloop(true);
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
        // Listen to incoming UART messages.
        if (!(i % CFG_TICK_FREQUENCY)) {
            uart_listen_char();
        }
        // Print additional timing data.
        if (CFG_LOG_LEVEL && !(i % 1000)) {
            info("Tick comp=%li idle=%i\n", tick_completed, tick_idle);
        }
        if (tick_idle > 0) {
            sleep_us((uint32_t)tick_idle);
        } else {
            info("+");
        }
        i++;
    }
}

int main() {
    main_init();
    main_loop();
}
