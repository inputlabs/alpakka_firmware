// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include <pico/stdlib.h>
#include <pico/time.h>
#include <tusb.h>
#include "config.h"
#include "tusb_config.h"
#include "bt_config.h"
#include "led.h"
#include "bus.h"
#include "profile.h"
#include "touch.h"
#include "imu.h"
#include "hid.h"
#include "uart.h"
#include "logging.h"
#include "helper.h"

#include "hci_dump.h"
#include "hci_dump_embedded_stdout.h"


#if __has_include("version.h")
    #include "version.h"
#else
    #define VERSION "undefined"
#endif

//#define ENABLE_HCI_DUMP


void title() {
    info("╔====================╗\n");
    info("║ Input Labs Oy.     ║\n");
    info("║ Alpakka controller ║\n");
    info("╚====================╝\n");
    info("Firmware version: %s\n", VERSION);
}

void main_init() {
    // LED feedback ASAP after booting.
    led_init();
    // Init stdio and logging.
    stdio_uart_init();
    stdio_init_all();

//    logging_set_level(LOG_INFO);
    logging_set_level(LOG_DEBUG);    
    logging_init();

    // Init USB.
    tusb_init();
    wait_for_usb_init();

    // Setup BT HID device
#ifdef ENABLE_HCI_DUMP 
    hci_dump_init(hci_dump_embedded_stdout_get_instance()); 
    hci_dump_enable_packet_log(true); 
#endif
    // BT Init
    bt_hid_device_setup();

    // Init components.
    title();
    config_init();
    bus_init();
    hid_init();
    thumbstick_init();
    touch_init();
    rotary_init();
    profile_init();
    imu_init();
}

void main_loop() {
    int16_t i = 0;
    logging_set_onloop(true);
    while (true) {
        i++;
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
        uart_listen_char(i);
        // Timing stats.
        if (logging_get_level() >= LOG_DEBUG) {
            static float average = 0;
            average = smooth(average, tick_completed, 100);
            if (!(i % 2000)) debug("Loop: avg=%.0f (us)\n", average);
        }
        // Idling control.
        if (tick_idle > 0) sleep_us((uint32_t)tick_idle);
        else info("+");
    }
}

int main() {
    main_init();
    main_loop();
}
