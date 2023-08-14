// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include <stdbool.h>
#include <tusb.h>
#include <device/usbd_pvt.h>
#include "webusb.h"
#include "tusb_config.h"
#include "helper.h"
#include "logging.h"

char webusb_buffer[1024] = {0,};
uint16_t webusb_ptr_in = 0;
uint16_t webusb_ptr_out = 0;
bool webusb_timedout = false;
bool webusb_buffer_lock = false;

void webusb_flush_force() {
    uint16_t i = 0;
    while(true) {
        busy_wait_ms(1);
        tud_task();
        bool sent = webusb_flush();
        if (sent) {
            break;
        }
        else {
            i++;
            if (i>500) {
                printf("USB: WebUSB not found\n");
                webusb_timedout = true;
                return;
            }
        }
    }
}

bool webusb_flush() {
    if (webusb_timedout) return true;
    if (webusb_ptr_in == 0) return true;
    if (!tud_ready() || usbd_edpt_busy(0, ADDR_WEBUSB_IN)) return false;
    char part[64] = {0,};
    uint16_t len = max(0, min(64, webusb_ptr_in - webusb_ptr_out));
    strncpy(part, webusb_buffer + webusb_ptr_out, len);
    webusb_ptr_out += len;
    if (webusb_ptr_out >= webusb_ptr_in) {
        webusb_ptr_in = 0;
        webusb_ptr_out = 0;
    }
    usbd_edpt_claim(0, ADDR_WEBUSB_IN);
    usbd_edpt_xfer(0, ADDR_WEBUSB_IN, (uint8_t*)part, len);
    usbd_edpt_release(0, ADDR_WEBUSB_IN);
    return true;
}

void webusb_write(char *msg) {
    if (webusb_timedout) return;
    uint16_t len = strlen(msg);
    if (webusb_ptr_in + len >= 1023-64) {
        printf("Warning: Full WebUSB buffer\n");
        return;
    }
    strncpy(webusb_buffer+webusb_ptr_in, msg, len);
    webusb_ptr_in += len;
    if (!logging_get_onloop()) {
        webusb_flush_force();
    }
}
