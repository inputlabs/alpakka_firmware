// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#pragma once

#define XINPUT_REPORT_SIZE 20

typedef struct __packed _XInputReport {
    uint8_t report_id;
    uint8_t report_size;
    uint8_t buttons_0;
    uint8_t buttons_1;
    uint8_t lz;
    uint8_t rz;
    int16_t lx;
    int16_t ly;
    int16_t rx;
    int16_t ry;
    uint8_t reserved[6];
} XInputReport;

bool xinput_send_report(XInputReport *report);
void xinput_receive_report();
