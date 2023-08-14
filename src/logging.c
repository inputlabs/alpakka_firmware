// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <tusb.h>
#include <device/usbd_pvt.h>
#include "tusb_config.h"
#include "logging.h"
#include "webusb.h"

bool logging_onloop = false;

void logging_set_onloop(bool value) {
    logging_onloop = value;
}

bool logging_get_onloop() {
    return logging_onloop;
}

void info(char *msg, ...) {
    va_list va;
    va_start(va, 0);
    char formatted[256] = {0,};
    vsnprintf(formatted, 256, msg, va);
    // UART.
    printf(formatted);
    // WebUSB.
    webusb_write(formatted);
}
