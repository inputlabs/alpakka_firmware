// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include <pico/stdlib.h>
#include <pico/time.h>
#include <pico/multicore.h>
#include <pico/flash.h>
#include <tusb.h>
#include "config.h"
#include "wireless.h"
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

void main_loop(); /////////

void host_init() {
    flash_safe_execute_core_init();
    stdio_uart_init();
    stdio_init_all();
    logging_set_level(LOG_INFO);
    logging_init();
    title();
    tusb_init();
    wait_for_usb_init();
    // config_init();
    hid_init();
    multicore_launch_core1(wireless_host_init);
    main_loop();
}

void host_task() {
    hid_report_from_queue();
    // tud_task();
    // if (tud_ready() && tud_hid_ready()) {
    //     webusb_read();
    //     webusb_flush();
    // }
}

void device_init() {
    flash_safe_execute_core_init();
    // LED feedback ASAP after booting.
    led_init();
    // Init stdio and logging.
    stdio_uart_init();
    stdio_init_all();
    logging_set_level(LOG_INFO);
    logging_init();
    // Load config.
    title();
    config_init();
    // Init USB.
    // tusb_init();
    // wait_for_usb_init();
    // Init components.
    bus_init();
    hid_init();
    thumbstick_init();
    touch_init();
    rotary_init();
    profile_init();
    imu_init();
    // MODE 0
        multicore_launch_core1(wireless_device_init);
        main_loop();
    // MODE 1
        // multicore_launch_core1(main_loop);
        // wireless_device_init();
}

void device_task() {
    config_sync();
    profile_report_active();
    // tud_task();
    // if (tud_ready() && tud_hid_ready()) {
    //     webusb_read();
    //     webusb_flush();
    // }
    hid_report_wireless();
}

void main_loop() {
    info("INIT: Main loop (core %i)\n", get_core_num());
    int16_t i = 0;
    logging_set_onloop(true);
    while (true) {
        i++;
        // Start timer.
        uint32_t tick_start = time_us_32();

        // Report.
        #ifdef FW_DEVICE_DONGLE
            host_task();
        #endif
        #ifdef FW_DEVICE_ALPAKKA
            device_task();
        #endif

        // Tick interval control.
        uint32_t tick_completed = time_us_32() - tick_start;
        uint16_t tick_interval = 1000000 / CFG_TICK_FREQUENCY;
        int32_t tick_idle = tick_interval - (int32_t)tick_completed;
        // Listen to incoming UART messages.
        uart_listen_char(i);
        // Timing stats.
        if (logging_get_level() >= LOG_DEBUG) {
            static float average = 0;
            static float max = 0;
            average += tick_completed;
            if (tick_completed > max) max = tick_completed;
            if (!(i % CFG_TICK_FREQUENCY)) {
                info("Loop: avg=%.0f max=%.0f\n", average/1000, max);
                average = max = 0;
            }
        }
        // Idling control.
        if (tick_idle > 0) sleep_us((uint32_t)tick_idle);
        // else info("+");
    }
}

int main() {
    #ifdef FW_DEVICE_ALPAKKA
        device_init();
    #endif
    #ifdef FW_DEVICE_DONGLE
        host_init();
    #endif
}
