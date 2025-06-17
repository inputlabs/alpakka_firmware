// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#pragma once
#include "ctrl.h"

#define BATTERY_MIN 2700
#define BATTERY_MAX 3350
#define BATTERY_LOW_THRESHOLD 2810
#define BATTERY_CAPACITY (BATTERY_MAX - BATTERY_MIN)

#define FAKE_PAIR_TIME_MS 2000

void wireless_init();
void wireless_controller_task();
void wireless_dongle_task();
void wireless_set_uart_data_mode(bool mode);

void wireless_send_hid(uint8_t report_id, void *packet, uint8_t len);
void wireless_send_webusb(Ctrl ctrl);
