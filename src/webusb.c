// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include <stdbool.h>
#include <tusb.h>
#include <device/usbd_pvt.h>
#include "webusb.h"
#include "config.h"
#include "hid.h"
#include "tusb_config.h"
#include "helper.h"
#include "logging.h"

char webusb_buffer[WEBUSB_BUFFER_SIZE] = {0,};
uint16_t webusb_ptr_in = 0;
uint16_t webusb_ptr_out = 0;
bool webusb_timedout = false;

void webusb_flush_force() {
    uint16_t i = 0;
    while(true) {
        tud_task();
        if (webusb_flush()) break;
        else {
            sleep_ms(1);
            i++;
            if (i>500) {
                // printf("USB: WebUSB timed out\n");
                webusb_timedout = true;
                return;
            }
        }
    }
}

bool webusb_flush() {
    // if (webusb_timedout) return true;
    if (webusb_ptr_in == 0) return true;
    if (!tud_ready() || usbd_edpt_busy(0, ADDR_WEBUSB_IN)) return false;
    uint16_t len = max(0, min(64, webusb_ptr_in - webusb_ptr_out));
    uint8_t* offset_ptr = webusb_buffer + webusb_ptr_out;
    usbd_edpt_claim(0, ADDR_WEBUSB_IN);
    usbd_edpt_xfer(0, ADDR_WEBUSB_IN, offset_ptr, len);
    usbd_edpt_release(0, ADDR_WEBUSB_IN);
    webusb_ptr_out += len;
    if (webusb_ptr_out >= webusb_ptr_in) {
        webusb_ptr_in = 0;
        webusb_ptr_out = 0;
    }
    return true;
}

void webusb_write(char *msg) {
    // if (webusb_timedout) return;
    uint16_t len = strlen(msg);
    if (webusb_ptr_in + len >= WEBUSB_BUFFER_SIZE-64-1) {
        printf("Warning: WebUSB buffer is full\n");
        return;
    }
    strncpy(webusb_buffer + webusb_ptr_in, msg, len);
    webusb_ptr_in += len;
    if (!logging_get_onloop()) {
        if (!webusb_timedout) {
            webusb_flush_force();
        }
    } else {
        webusb_timedout = false;
    }
}

void webusb_handle_proc(uint8_t proc) {
    if (proc == PROC_RESTART) config_reboot();
    else if (proc == PROC_BOOTSEL) config_bootsel();
    else if (proc == PROC_CALIBRATE) config_calibrate();
    else if (proc == PROC_FACTORY) config_factory();
}

void webusb_read() {
    if (!tud_ready() || usbd_edpt_busy(0, ADDR_WEBUSB_OUT)) return;
    Ctrl message[8];
    usbd_edpt_claim(0, ADDR_WEBUSB_OUT);
    usbd_edpt_xfer(0, ADDR_WEBUSB_OUT, (uint8_t*)message, 64);
    usbd_edpt_release(0, ADDR_WEBUSB_OUT);
    if (
        message->protocol_version == 1 &&
        message->device_id == 1 &&
        message->message_type == 2
    ) {
        // info("CTRL: Message received type=%i\n", message->message_type);
        webusb_handle_proc(message->payload[0]);
    }
}
