// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#pragma once
#include <stdint.h>

#define WEBUSB_BUFFER_SIZE 2048

typedef struct Ctrl_struct {
    uint8_t protocol_version;
    uint8_t device_id;
    uint8_t message_type;
    uint8_t payload[61];
} Ctrl;

void webusb_read();
void webusb_write(char *msg);
bool webusb_flush();
void webusb_flush_force();
