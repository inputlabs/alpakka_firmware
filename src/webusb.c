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
#include "power.h"
#include "loop.h"
#include "wireless.h"

char webusb_buffer[WEBUSB_BUFFER_SIZE] = {0,};
uint16_t webusb_ptr_in = 0;
uint16_t webusb_ptr_out = 0;
bool webusb_timedout = false;

static bool webusb_pending_empty = false;
static bool webusb_pending_status_share = false;
static uint8_t webusb_pending_config_share = 0;
static uint8_t webusb_pending_profile_share = 0;
static uint8_t webusb_pending_section_share = 0;

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

bool webusb_transfer_wired(Ctrl ctrl) {
    // Check if TinyUSB device is ready (connected).
    if (!tud_ready()) return false;
    // Check if USB endpoint is free.
    if (usbd_edpt_busy(0, ADDR_WEBUSB_IN)) return false;
    // Claim USB endpoint.
    if (!usbd_edpt_claim(0, ADDR_WEBUSB_IN)) return false;
    // Transfer data.
    bool success = usbd_edpt_xfer(0, ADDR_WEBUSB_IN, (char*)&ctrl, ctrl.len+4);
    // Release USB endpoint.
    usbd_edpt_release(0, ADDR_WEBUSB_IN);
    return success;
}

bool webusb_transfer(Ctrl ctrl) {
    #if defined DEVICE_IS_ALPAKKA
        if (loop_get_device_mode() == WIRED) {
            return webusb_transfer_wired(ctrl);
        }
        if (loop_get_device_mode() == WIRELESS) {
            wireless_send_webusb(ctrl);
            return true;
        }
    #elif defined DEVICE_DONGLE
        return webusb_transfer_wired(ctrl);
    #endif
}

bool webusb_flush() {
    // Check if there is anything to flush.
    if (
        webusb_ptr_in == 0 &&
        !webusb_pending_status_share &&
        !webusb_pending_config_share &&
        !webusb_pending_profile_share &&
        !webusb_pending_section_share
    ) {
        return true;
    }
    // Using static to ensure the variable lives long enough in memory to be
    // referenced by the transfer underlying mechanisms.
    static Ctrl ctrl;
    // Generate message.
    if (webusb_pending_empty) {
        ctrl = ctrl_empty();
        bool sent = webusb_transfer(ctrl);
        if (sent) webusb_pending_empty = false;
    } else if (webusb_pending_status_share) {
        ctrl = ctrl_status_share();
        bool sent = webusb_transfer(ctrl);
        if (sent) webusb_pending_status_share = false;
    } else if (webusb_pending_config_share) {
        ctrl = ctrl_config_share(webusb_pending_config_share);
        bool sent = webusb_transfer(ctrl);
        if (sent) webusb_pending_config_share = 0;
    } else if (webusb_pending_profile_share || webusb_pending_section_share) {
        ctrl = ctrl_section_share(webusb_pending_profile_share, webusb_pending_section_share);
        bool sent = webusb_transfer(ctrl);
        if (sent) {
            webusb_pending_profile_share = 0;
            webusb_pending_section_share = 0;
        }
    } else {
        uint8_t len = constrain(webusb_ptr_in-webusb_ptr_out, 0, CTRL_MAX_PAYLOAD_SIZE);
        uint8_t *offset_ptr = webusb_buffer + webusb_ptr_out;
        ctrl = ctrl_log(offset_ptr, len);
        bool sent = webusb_transfer(ctrl);
        if (sent) {
            webusb_ptr_out += len;
            if (webusb_ptr_out >= webusb_ptr_in) {
                webusb_ptr_in = 0;
                webusb_ptr_out = 0;
            }
        }
    }
    return true;
}

// Queue data to be sent (flushed) to the app later.
void webusb_write(char *msg) {
    uint16_t len = strlen(msg);
    // If the buffer is full, ignore the latest messages.
    if (webusb_ptr_in + len >= WEBUSB_BUFFER_SIZE-64-1) {
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

void webusb_handle_status_get() {
    debug("WebUSB: Received status GET from app\n");
    webusb_pending_empty = true;
    webusb_pending_status_share = true;
}

void webusb_handle_status_set(uint8_t time[8]) {
    debug("WebUSB: Received status SET from app\n");
    // set_system_clock(*(uint64_t*)time);  // TODO: Backport from other branch.
}

void webusb_handle_proc(uint8_t proc) {
    if (proc == PROC_RESTART) power_restart();
    else if (proc == PROC_BOOTSEL) power_bootsel();
    else if (proc == PROC_CALIBRATE) config_calibrate();
    else if (proc == PROC_RESET_FACTORY) config_reset_factory();
    else if (proc == PROC_RESET_CONFIG) config_reset_config();
    else if (proc == PROC_RESET_PROFILES) config_reset_profiles();
}

void webusb_handle_config_get(Ctrl_cfg_type key) {
    webusb_pending_config_share = key;
}

void webusb_handle_section_get(uint8_t profile, uint8_t section) {
    webusb_pending_profile_share = profile;
    webusb_pending_section_share = section;
}

void webusb_handle_section_set(uint8_t profileIndex, uint8_t sectionIndex, uint8_t section[58]) {
    debug("WebUSB: Handle profile SET %i %i\n", profileIndex, sectionIndex);
    // Update profile in config.
    CtrlProfile *profile_cfg = config_profile_read(profileIndex);
    profile_cfg->sections[sectionIndex] = *(CtrlSection*)section;
    // Update profile runtime.
    Profile *profile = profile_get(profileIndex);
    profile->load_from_config(profile, profile_cfg);
    config_profile_set_sync(profileIndex, false);
    // Send back data as confirmation.
    webusb_pending_profile_share = profileIndex;
    webusb_pending_section_share = sectionIndex;
}

// Handle incomming message.
void webusb_handle(Ctrl ctrl) {
    if (ctrl.message_type == PROC) webusb_handle_proc(ctrl.payload[0]);
    if (ctrl.message_type == STATUS_GET) webusb_handle_status_get();
    if (ctrl.message_type == STATUS_SET) webusb_handle_status_set(ctrl.payload);
    if (ctrl.message_type == CONFIG_GET) webusb_handle_config_get(ctrl.payload[0]);
    if (ctrl.message_type == CONFIG_SET) {
        webusb_pending_config_share = ctrl.payload[0];  // Echo / confirmation.
        ctrl_config_set(
            ctrl.payload[0],  // Config index.
            ctrl.payload[1],  // Preset index.
            &ctrl.payload[2]  // Preset values. (Reference to sub-array).
        );
    }
    if (ctrl.message_type == SECTION_GET) {
        webusb_handle_section_get(ctrl.payload[0], ctrl.payload[1]);
    }
    if (ctrl.message_type == SECTION_SET) {
        webusb_handle_section_set(
            ctrl.payload[0],
            ctrl.payload[1],
            &ctrl.payload[2]
        );
    }
    if (ctrl.message_type == PROFILE_OVERWRITE) {
        config_profile_overwrite(ctrl.payload[0], ctrl.payload[1]);
    }
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
    // Wireless switch.
    if (ctrl.protocol_flags == CTRL_FLAG_WIRELESS) {
        // Redirect to wireless controller.
        wireless_send_webusb(ctrl);
    } else {
        // Handle locally.
        webusb_handle(ctrl);
    }
}

void webusb_set_pending_config_share(bool value) {
    webusb_pending_config_share = value;
}
