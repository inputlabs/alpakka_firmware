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

bool webusb_onloop = false;

void webusb_set_onloop(bool value) {
    webusb_onloop = value;
}

bool webusb_send(char *msg) {
    if (tud_ready() && !usbd_edpt_busy(0, ADDR_WEBUSB_IN)) {
        usbd_edpt_claim(0, ADDR_WEBUSB_IN);
        usbd_edpt_xfer(0, ADDR_WEBUSB_IN, (uint8_t*)msg, 64);
        usbd_edpt_release(0, ADDR_WEBUSB_IN);
        return true;
    }
    return false;
}

void webusb_send_onloop(char *msg) {
    while(true) {
        tud_task();
        bool sent = webusb_send(msg);
        if (sent) break;
        else sleep_ms(1);
    }
    sleep_ms(1);
}

void info(char *msg, ...) {
    va_list va;
    // UART.
    va_start(va, 0);
    vprintf(msg, va);
    // WebUSB.
    va_start(va, 0);
    char formatted[64] = {0,};
    vsnprintf(formatted, 64, msg, va);
    if (webusb_onloop) webusb_send(formatted);
    else webusb_send_onloop(formatted);
}


