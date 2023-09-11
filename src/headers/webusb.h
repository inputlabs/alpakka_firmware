// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#pragma once
#include <stdint.h>

#define WEBUSB_BUFFER_SIZE 2048

typedef enum Ctrl_msg_type_enum {
    LOG = 1,
    PROC,
    CONFIG_GET,
    CONFIG_SET,
    CONFIG_GIVE,
} Ctrl_msg_type;

typedef enum Ctrl_cfg_type_enum {
    PROTOCOL = 1,
    SENS_TOUCH,
    SENS_MOUSE,
    DEADZONE,
} Ctrl_cfg_type;

typedef struct Ctrl_struct {
    uint8_t protocol_version;
    uint8_t device_id;
    Ctrl_msg_type message_type;
    uint8_t len;
    uint8_t payload[60];
} Ctrl;

void webusb_read();
void webusb_write(char *msg);
bool webusb_flush();
void webusb_flush_force();
