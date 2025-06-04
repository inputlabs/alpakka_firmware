// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#pragma once
#include <stdint.h>
#include <stdbool.h>

#define LABEL_CONTROLLER "Alpakka controller"
#define LABEL_DONGLE     "Wireless dongle   "
#define USB_WAIT_FOR_INIT_MS 1000  // 1 second.
#define USB_DONGLE_CHECK_US 2000000  // 2 seconds.

#if defined DEVICE_ALPAKKA_V1
    #define REPORT_TIMEOUT_US 500000  // 0.5 seconds.
#else
    #define REPORT_TIMEOUT_US 10000000  // 10 seconds.
#endif

typedef enum {
    WIRED,
    WIRELESS,
    INACTIVE,
} DeviceMode;

uint64_t has_system_clock();
uint64_t get_system_clock();
void set_system_clock(uint64_t time);

DeviceMode loop_get_device_mode();
void loop_set_battery_low(bool state);

void loop_controller_init();
void loop_dongle_init();
void loop_llama_init();
void loop_run();
