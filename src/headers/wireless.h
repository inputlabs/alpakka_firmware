// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#pragma once
#include <stdint.h>
#include <pico/cyw43_arch.h>
#include "btstack_config.h"

#define DEVICE_NAME "AE2WE2XA4T"
#define DEVICE_NAME_AND_MAC "AE2WE2XA4T 00:00:00:00:00:00"
#define CLASS_OF_DEVICE 0x1701
#define RFCOMM_CHANNEL 1
#define PACKET_MTU 48  // Bytes.
#define POWER_MANAGEMENT_SLEEP_TIMEOUT 2000  // Milliseconds.

#define DEVICE_SPP_SERVICE_BUFFER_SIZE 150
#define DEVICE_TASK_INTERVAL 1  // Milliseconds.

#define DONGLE_INQUIRY_INTERVAL 5  // Seconds.
#define DONGLE_TASK_INTERVAL 4  // Milliseconds.
#define DONGLE_LED_INTERVAL_CONNECTED 500  // Milliseconds.
#define DONGLE_LED_INTERVAL_DISCONNECTED 100  // Milliseconds.

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
