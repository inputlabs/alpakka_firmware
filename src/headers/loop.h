// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#pragma once
#include <stdint.h>

#define USB_WAIT_FOR_INIT_MS 1000

typedef enum {
    WIRED,
    WIRELESS,
    DORMANT,
} DeviceMode;

uint64_t has_system_clock();
uint64_t get_system_clock();
void set_system_clock(uint64_t time);

DeviceMode loop_get_device_mode();
void loop_device_init();
void loop_host_init();
void loop_cycle();
