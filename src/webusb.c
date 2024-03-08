// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include <stdbool.h>
#include <tusb.h>
#include <device/usbd_pvt.h>
#include "webusb.h"
#include "ctrl.h"
#include "config.h"
#include "profile.h"
#include "hid.h"
#include "tusb_config.h"
#include "common.h"
#include "logging.h"

char webusb_buffer[WEBUSB_BUFFER_SIZE] = {0,};
uint16_t webusb_ptr_in = 0;
uint16_t webusb_ptr_out = 0;
bool webusb_timedout = false;

uint8_t webusb_pending_establish = 1;
uint8_t webusb_pending_config_share = 0;
uint8_t webusb_pending_profile_share = 0;
uint8_t webusb_pending_section_share = 0;

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
        !webusb_pending_establish &&
        !webusb_pending_config_share &&
        !webusb_pending_profile_share &&
        !webusb_pending_section_share
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
    if (webusb_pending_establish) {
        ctrl = ctrl_establish_device();
        webusb_pending_establish = 0;
    } else if (webusb_pending_config_share) {
        ctrl = ctrl_config_share(webusb_pending_config_share);
        webusb_pending_config_share = 0;
    } else if (webusb_pending_profile_share || webusb_pending_section_share) {
        ctrl = ctrl_profile_share(webusb_pending_profile_share, webusb_pending_section_share);
        webusb_pending_profile_share = 0;
        webusb_pending_section_share = 0;
    } else {
        uint8_t len = constrain(webusb_ptr_in-webusb_ptr_out, 0, CTRL_MAX_PAYLOAD_SIZE);
        uint8_t *offset_ptr = webusb_buffer + webusb_ptr_out;
        ctrl = ctrl_log(offset_ptr, len);
        webusb_ptr_out += len;
        if (webusb_ptr_out >= webusb_ptr_in) {
            webusb_ptr_in = 0;
            webusb_ptr_out = 0;
        }
    }
    // Transfer message.
    usbd_edpt_xfer(0, ADDR_WEBUSB_IN, (unsigned char *)&ctrl, ctrl.len+4);
    usbd_edpt_release(0, ADDR_WEBUSB_IN);
    return true;
}

// Queue data to be sent (flushed) to the app later.
void webusb_write(char *msg) {
    uint16_t len = strlen(msg);
    // If the buffer is full, ignore the latest messages.
    if (webusb_ptr_in + len >= WEBUSB_BUFFER_SIZE-64-1) {
//        printf("Warning: WebUSB buffer is full\n");
        return;
    }
    // Add message to the buffer.
    strncpy(webusb_buffer + webusb_ptr_in, msg, len);
    webusb_ptr_in += len;
    // If the configuration is still running (still not in the main loop), and
    // the webusb connection has not been flagged as timed out, then force
    // flush directly.
    if (!logging_get_onloop()) {
        if (!webusb_timedout) {
            webusb_flush_force();
        }
    } else {
        webusb_timedout = false;
    }
}

void webusb_handle_establish(uint8_t time[8]) {
    uint64_t clock = 0;
    for (uint8_t i=0; i<8; i++) {
        clock += ((uint64_t)time[i]) << (8 * i);
    }
    set_system_clock(clock);
}

void webusb_handle_proc(uint8_t proc) {
    if (proc == PROC_RESTART) config_reboot();
    else if (proc == PROC_BOOTSEL) config_bootsel();
    else if (proc == PROC_CALIBRATE) config_calibrate();
    else if (proc == PROC_FACTORY) config_factory();
}

void webusb_handle_config_get(Ctrl_cfg_type key) {
    webusb_pending_config_share = key;
}

void webusb_handle_profile_get(uint8_t profile, uint8_t section) {
    webusb_pending_profile_share = profile;
    webusb_pending_section_share = section;
}

void webusb_handle_config_set(Ctrl_cfg_type key, uint8_t preset, uint8_t values[5]) {
    if (key > 4) return;
    webusb_pending_config_share = key;
    if (key == PROTOCOL) config_set_protocol(preset);
    else if (key == SENS_TOUCH) {
        config_set_touch_sens_values(values);
        config_set_touch_sens_preset(preset, false);
    }
    else if (key == SENS_MOUSE) {
        // Scaled by 10 since the USB communication works with integers.
        double values_fmt[] = {
            values[0] / 10.0,
            values[1] / 10.0,
            values[2] / 10.0
            // Indexes 3 and 4 are ignored.
        };
        config_set_mouse_sens_values(values_fmt);
        config_set_mouse_sens_preset(preset, false);
    }
    else if (key == DEADZONE) {
        // Scaled by 100 so the USB communication works with integers.
        float values_fmt[] = {
            values[0] / 100.0,
            values[1] / 100.0,
            values[2] / 100.0
            // Indexes 3 and 4 are ignored.
        };
        config_set_deadzone_values(values_fmt);
        config_set_deadzone_preset(preset, false);
    }
}

void webusb_handle_profile_set(uint8_t profileIndex, uint8_t sectionIndex, uint8_t section[58]) {
    // Update profile in config.
    CtrlProfile *profile_cfg = config_profile_read(profileIndex);
    profile_cfg->sections[sectionIndex] = *(CtrlSection*)section;
    // Update profile runtime.
    Profile *profile = profile_get(profileIndex);
    profile->load_from_config(profile, profile_cfg);
    config_profile_set_sync(profileIndex, false);
}

void webusb_read() {
    // Parse data coming from the app.
    if (!tud_ready() || usbd_edpt_busy(0, ADDR_WEBUSB_OUT)) return;
    // Using static to ensure the variable lives long enough in memory to be
    // referenced by the transfer underlying mechanisms.
    static Ctrl ctrl;
    usbd_edpt_claim(0, ADDR_WEBUSB_OUT);
    usbd_edpt_xfer(0, ADDR_WEBUSB_OUT, (uint8_t*)&ctrl, 64);
    usbd_edpt_release(0, ADDR_WEBUSB_OUT);
    // Handle incomming message.
    if (ctrl.message_type == ESTABLISH_HOST) webusb_handle_establish(ctrl.payload);
    if (ctrl.message_type == PROC) webusb_handle_proc(ctrl.payload[0]);
    if (ctrl.message_type == CONFIG_GET) webusb_handle_config_get(ctrl.payload[0]);
    if (ctrl.message_type == CONFIG_SET) {
        webusb_handle_config_set(
            ctrl.payload[0],  // Config index.
            ctrl.payload[1],  // Preset index.
            &ctrl.payload[2]  // Preset values. (Reference to sub-array).
        );
    }
    if (ctrl.message_type == PROFILE_GET) {
        webusb_handle_profile_get(ctrl.payload[0], ctrl.payload[1]);
    }
    if (ctrl.message_type == PROFILE_SET) {
        webusb_handle_profile_set(
            ctrl.payload[0],
            ctrl.payload[1],
            &ctrl.payload[2]
        );
    }
}

void webusb_set_pending_config_share(bool value) {
    webusb_pending_config_share = value;
}
