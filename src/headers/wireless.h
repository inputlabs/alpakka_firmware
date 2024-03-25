// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#pragma once
#include <stdint.h>
#include <pico/cyw43_arch.h>
#include "btstack_config.h"

#define DEVICE_NAME "Alpakka wireless"
#define DEVICE_NAME_AND_MAC "Alpakka wireless 00:00:00:00:00:00"
#define CLASS_OF_DEVICE 0x1701
#define INQUIRY_INTERVAL 5
#define RFCOMM_CHANNEL 1
#define OFFSTREAM_INTERVAL 1  // Milliseconds.

typedef enum {
    SCANNING,
    SCAN_COMPLETE,
    QUERYING,
    CONNECTING,
    CONNECTED,
} host_state_t;

void host_task();
void device_task();

void wireless_host_init();
void wireless_device_init();

bool wireless_device_is_connected();

void wireless_report_keyboard(int8_t modifier, int8_t keys[6]);
void wireless_report_mouse(int8_t buttons, int16_t x, int16_t y);
void wireless_task();
