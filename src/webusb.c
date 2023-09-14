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
Ctrl_cfg_type webusb_pending_config_give = 0;

Ctrl webusb_ctrl_log() {
    Ctrl ctrl = {
        .protocol_version = 1,
        .device_id = 1,
        .message_type = LOG
    };
    ctrl.len = max(0, min(64-4, webusb_ptr_in - webusb_ptr_out));
    uint8_t* offset_ptr = webusb_buffer + webusb_ptr_out;
    for (uint8_t i=0; i<ctrl.len; i++) {
        ctrl.payload[i] = offset_ptr[i];
    }
    webusb_ptr_out += ctrl.len;
    if (webusb_ptr_out >= webusb_ptr_in) {
        webusb_ptr_in = 0;
        webusb_ptr_out = 0;
    }
    return ctrl;
}

Ctrl webusb_ctrl_config_give() {
    Ctrl ctrl = {
        .protocol_version = 1,
        .device_id = 1,
        .message_type = CONFIG_GIVE,
        .len = 2
    };
    ctrl.payload[0] = webusb_pending_config_give;
    if      (webusb_pending_config_give == PROTOCOL)   ctrl.payload[1] = config_get_protocol();
    else if (webusb_pending_config_give == SENS_TOUCH) ctrl.payload[1] = config_get_touch_sens();
    else if (webusb_pending_config_give == SENS_MOUSE) ctrl.payload[1] = config_get_mouse_sens();
    else if (webusb_pending_config_give == DEADZONE)   ctrl.payload[1] = config_get_deadzone();
    webusb_pending_config_give = 0;
    return ctrl;
}

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
    // Check if there is anything to flush.
    if (
        webusb_ptr_in == 0 &&
        !webusb_pending_config_give
    ) {
        return true;
    }
    // Check if the WebUSB interface/endpoint is ready.
    if (
        !tud_ready() ||
        usbd_edpt_busy(0, ADDR_WEBUSB_IN) ||
        !usbd_edpt_claim(0, ADDR_WEBUSB_IN)
    ) {
        return false;
    }
    // Using static to ensure the variable lives long enough in memory to be
    // referenced by the transfer underlying mechanisms.
    static Ctrl ctrl;
    // Generate message.
    if (webusb_pending_config_give) ctrl = webusb_ctrl_config_give();
    else ctrl = webusb_ctrl_log();
    // Transfer message.
    usbd_edpt_xfer(0, ADDR_WEBUSB_IN, (unsigned char *)&ctrl, ctrl.len+4);
    usbd_edpt_release(0, ADDR_WEBUSB_IN);
    return true;
}

void webusb_write(char *msg) {
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

void webusb_handle_config_get(Ctrl_cfg_type key) {
    webusb_pending_config_give = key;
}

void webusb_handle_config_set(Ctrl_cfg_type key, uint8_t preset) {
    if (key > 4) return;
    webusb_pending_config_give = key;
    if      (key == PROTOCOL)   config_set_protocol(preset);
    else if (key == SENS_TOUCH) config_set_touch_sens(preset);
    else if (key == SENS_MOUSE) config_set_mouse_sens(preset);
    else if (key == DEADZONE)   config_set_deadzone(preset);
}

void webusb_read() {
    if (!tud_ready() || usbd_edpt_busy(0, ADDR_WEBUSB_OUT)) return;
    Ctrl message[8];
    usbd_edpt_claim(0, ADDR_WEBUSB_OUT);
    usbd_edpt_xfer(0, ADDR_WEBUSB_OUT, (uint8_t*)message, 64);
    usbd_edpt_release(0, ADDR_WEBUSB_OUT);
    // Handle incomming messages.
    if (message->message_type == PROC) {
        webusb_handle_proc(message->payload[0]);
    }
    if (message->message_type == CONFIG_GET) {
        webusb_handle_config_get(message->payload[0]);
    }
    if (message->message_type == CONFIG_SET) {
        webusb_handle_config_set(message->payload[0], message->payload[1]);
    }
}

void webusb_set_pending_config_give(bool value) {
    webusb_pending_config_give = value;
}
