// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#pragma once
#include <stdint.h>
#include <pico/cyw43_arch.h>
#include "btstack_config.h"

#define REPORT_ID 0x01

static enum {
    APP_BOOTING,
    APP_NOT_CONNECTED,
    APP_CONNECTING,
    APP_CONNECTED
} app_state = APP_BOOTING;

void host_task();
void device_task();

void wireless_host_init();
void wireless_client_init();

void wireless_report_keyboard(int8_t modifier, int8_t keys[6]);
void wireless_report_mouse(int8_t buttons, int16_t x, int16_t y);
void wireless_task();
