// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#pragma once

// typedef struct _WLANKeyboardReport {
//     uint8_t report_type;
//     uint8_t modifiers;
//     uint8_t keys[6];
// }
// __attribute__((packed))
// WLANKeyboardReport;

// typedef struct _WLANMouseReport {
//     uint8_t report_type;
//     uint8_t buttons;
//     int16_t x;
//     int16_t y;
//     int8_t scroll;
// }
// __attribute__((packed))
// WLANMouseReport;

void wlan_host_setup();
void wlan_host_task();
void wlan_device_setup();
void wlan_device_task();

void wlan_report(uint8_t *report, uint8_t len);
void wlan_report_keyboard(uint8_t modifiers, uint8_t k[6]);
void wlan_report_mouse(int8_t buttons, int8_t x, int8_t y);
