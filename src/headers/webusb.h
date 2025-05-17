// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#pragma once
#include <stdbool.h>
#include "common.h"
#include "ctrl.h"

#define WEBUSB_BUFFER_SIZE 2048

void webusb_read();
void webusb_write(char *msg);
bool webusb_flush();
void webusb_flush_force();
void webusb_set_pending_config_share(bool value);
bool webusb_transfer_wired(Ctrl ctrl);
void webusb_handle(Ctrl ctrl);
