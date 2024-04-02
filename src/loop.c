// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include <pico/stdlib.h>
#include <pico/time.h>
#include <pico/multicore.h>
// #include <pico/flash.h>
// #include <hardware/flash.h>
#include <tusb.h>
#include "loop.h"
#include "config.h"
#include "wlan.h"
#include "led.h"
#include "bus.h"
#include "profile.h"
#include "touch.h"
#include "imu.h"
#include "hid.h"
#include "uart.h"
#include "logging.h"
#include "version.h"
#include "common.h"

static DeviceMode device_mode = WIRED;
static uint64_t system_clock = 0;

DeviceMode loop_get_device_mode() {
    return device_mode;
}

uint64_t has_system_clock() {
    return !!system_clock;
}

uint64_t get_system_clock() {
    return system_clock + (time_us_64() / 1000);
}

void set_system_clock(uint64_t time) {
    system_clock = time - (time_us_32() / 1000);
    info("LOOP: System_clock=%llu\n", system_clock);
}

void wait_for_system_clock() {
    info("Waiting for system clock\n");
    while (!has_system_clock()) {
        tud_task();
        if (tud_ready()) {
            webusb_read();
            webusb_flush();
        }
        sleep_ms(1);
    }
}

static void device_title() {
    info("╔====================╗\n");
    info("║ Input Labs Oy.     ║\n");
    info("║ Alpakka controller ║\n");
    info("╚====================╝\n");
    info("Firmware version: %s\n", VERSION);
}

static void dongle_title() {
    info("╔====================╗\n");
    info("║ Input Labs Oy.     ║\n");
    info("║ Wireless dongle    ║\n");
    info("╚====================╝\n");
    info("Firmware version: %s\n", VERSION);
}

// static void set_wired() {
//     return;
//     info("LOOP: Wired\n");
//     if (device_mode != WIRED) config_reboot();
//     device_mode = WIRED;
// }

// static void set_wireless() {
//     return;
//     info("LOOP: Wireless\n");
//     if (device_mode != WIRELESS) multicore_launch_core1(wireless_device_init);
//     device_mode = WIRELESS;
// }

void loop_device_init() {
    debug("LOOP: loop_device_init\n");
    // flash_safe_execute_core_init();
    led_init();
    stdio_uart_init();
    stdio_init_all();
    logging_set_level(LOG_INFO);
    logging_init();
    device_title();
    config_init();
    // tusb_init();
    // bool usb = usb_wait_for_init(USB_WAIT_FOR_INIT_MS);
    // wait_for_system_clock();
    bus_init();
    hid_init();
    thumbstick_init();
    touch_init();
    rotary_init();
    profile_init();
    imu_init();
    // if (0 && usb) set_wired();
    // else set_wireless();
    wlan_client_init();
    loop_cycle();
}

void loop_device_task() {
    // Write flash if needed.
    config_sync();
    // Gather values for input sources.
    profile_report_active();
    hid_report_wireless();
    wlan_client_task();
    // Report to the correct channel.
    // if (device_mode == WIRED) {
    //     // Report to USB.
    //     bool reported = hid_report();
    //     // Switch to wireless if USB is disconnected.
    //     if (!reported) set_wireless();
    // }
    // if (device_mode == WIRELESS) {
    //     // Report to wireless.
    //     hid_report_wireless();
    //     // Switch to wired if USB is connected (check once per second).
    //     static uint16_t i = 0;
    //     i++;
    //     if ((!(i % CFG_TICK_FREQUENCY)) && usb_is_connected()) set_wired();
    // }
    // Listen to UART commands.
    uart_listen();
}

void loop_host_init() {
    debug("LOOP: loop_host_init\n");
    // flash_safe_execute_core_init();
    stdio_uart_init();
    stdio_init_all();
    logging_set_level(LOG_INFO);
    logging_init();
    dongle_title();
    tusb_init();
    usb_wait_for_init(-1);  // Negative number = no timeout.
    // wait_for_system_clock();
    // config_init();
    hid_init();
    // multicore_launch_core1(wireless_host_init);
    wlan_server_init();
    loop_cycle();
}

void loop_host_task(uint16_t i) {
    wlan_server_task();
    hid_report_from_queue(i % 2);
    tud_task();
    if (tud_ready()) {
        webusb_read();
        webusb_flush();
    }
}

void loop_cycle() {
    info("LOOP: Main loop start (core %i)\n", get_core_num());
    uint16_t i = 0;
    logging_set_onloop(true);
    while (true) {
        i++;
        // Start timer.
        uint32_t start = time_us_32();
        // Task.
        #ifdef FW_DEVICE_ALPAKKA
            loop_device_task();
        #endif
        #ifdef FW_DEVICE_DONGLE
            loop_host_task(i);
        #endif
        // Calculate used time.
        uint32_t used = time_us_32() - start;
        int32_t unused = CFG_TICK_INTERVAL_IN_US - (int32_t)used;
        // Timing stats.
        if (logging_get_level() >= LOG_DEBUG) {
            static float average = 0;
            static float max = 0;
            average += used;
            if (used > max) max = used;
            if (!(i % CFG_TICK_FREQUENCY)) {
                info("Loop: avg=%.0f max=%.0f\n", average/1000, max);
                average = max = 0;
            }
        }
        // Idling control.
        if (unused > 0) sleep_us((uint32_t)unused);
        // else info("+");
    }
}
