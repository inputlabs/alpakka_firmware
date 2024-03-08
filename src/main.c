// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include <pico/stdlib.h>
#include <pico/time.h>
// #include "pico/multicore.h"
#include <tusb.h>
#include "config.h"
#include "tusb_config.h"
#include "wlan.h"
#include "led.h"
#include "bus.h"
#include "profile.h"
#include "touch.h"
#include "imu.h"
#include "hid.h"
#include "uart.h"
#include "logging.h"
#include "common.h"

#if __has_include("version.h")
    #include "version.h"
#else
    #define VERSION "undefined"
#endif

uint64_t system_clock = 0;

uint64_t get_system_clock() {
    return system_clock;
}

void set_system_clock(uint64_t time) {
    system_clock = time - (time_us_32() / 1000);
    info("INIT: System_clock=%llu\n", system_clock);
}

void title() {
    info("╔====================╗\n");
    info("║ Input Labs Oy.     ║\n");
    info("║ Alpakka controller ║\n");
    info("╚====================╝\n");
    info("Firmware version: %s\n", VERSION);
}

void title_dongle() {
    info("╔====================╗\n");
    info("║ Input Labs Oy.     ║\n");
    info("║ Wireless dongle    ║\n");
    info("╚====================╝\n");
    info("Firmware version: %s\n", VERSION);
}

void host_init() {
    stdio_uart_init();
    stdio_init_all();
    logging_set_level(LOG_INFO);
    logging_init();
    title_dongle();
    tusb_init();
    wait_for_usb_init();
    // config_init();
    hid_init();
    wlan_host_setup();
}

void device_init() {
    led_init();
    stdio_uart_init();
    stdio_init_all();
    logging_set_level(LOG_INFO);
    logging_init();
    title();
    config_init();
    tusb_init();
    wait_for_usb_init();
    // while(!get_system_clock()) {
    //     if (!get_system_clock()) {
    //         tud_task();
    //         if (tud_ready() && tud_hid_ready()) {
    //             webusb_read();
    //             webusb_flush();
    //         }
    //     }
    //     sleep_ms(1);
    // }
    bus_init();
    hid_init();
    thumbstick_init();
    touch_init();
    rotary_init();
    profile_init();
    imu_init();
    wlan_device_setup();
}

void host_task() {
    // hid_report();
    wlan_host_task();

    // WebUSB.
    tud_task();
    if (tud_ready() && tud_hid_ready()) {
        webusb_read();
        webusb_flush();
    }
}

void device_task() {
    config_sync();
    profile_report_active();
    wlan_device_task();
}

void main_loop() {
    info("INIT: Main loop\n");
    int16_t i = 0;
    logging_set_onloop(true);
    while (true) {
        i++;
        // Start timer.
        uint32_t tick_start = time_us_32();

        // Report.
        #ifdef FW_DEVICE_ALPAKKA
            device_task();
        #endif
        #ifdef FW_DEVICE_DONGLE
            host_task();
        #endif

        // Tick interval control.
        uint32_t tick_completed = time_us_32() - tick_start;
        uint32_t tick_interval = CFG_TICK_INTERVAL_US;
        int32_t tick_idle = tick_interval - (int32_t)tick_completed;
        // Listen to incoming UART messages.
        uart_listen_char(i);
        // Timing stats.
        if (1 || logging_get_level() >= LOG_DEBUG) {
            static float average = 0;
            average = smooth(average, tick_completed, 100);
            if (!(i % 1000)) info("Loop: avg=%.0f (us)\n", average);
        }
        // Idling control.
        if (tick_idle > 0) sleep_us((uint32_t)tick_idle);
        else info("+");
    }
}

int main() {
    #ifdef FW_DEVICE_ALPAKKA
        device_init();
        main_loop();
    #endif
    #ifdef FW_DEVICE_DONGLE
        host_init();
        main_loop();
    #endif
}
