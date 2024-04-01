// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <tusb_config.h>
#include <tusb.h>
#include "config.h"
#include "hid.h"
#include "led.h"
#include "logging.h"

static const char *const descriptor_string[] = {
    (const char[]){0x09, 0x04},  // English.
    STRING_VENDOR,
    STRING_PRODUCT,
    STRING_DEVICE_VERSION,
    STRING_HID,
    STRING_WEBUSB,
    STRING_XINPUT
};

uint8_t const descriptor_report_generic[] = {
    TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(REPORT_KEYBOARD)),
    TUD_HID_REPORT_DESC_MOUSE_CUSTOM(HID_REPORT_ID(REPORT_MOUSE)),
    TUD_HID_REPORT_DESC_GAMEPAD_CUSTOM(HID_REPORT_ID(REPORT_GAMEPAD)),
};

uint8_t const descriptor_report_xinput[] = {
    TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(REPORT_KEYBOARD)),
    TUD_HID_REPORT_DESC_MOUSE_CUSTOM(HID_REPORT_ID(REPORT_MOUSE)),
};

uint8_t descriptor_configuration_generic[] = {
    DESCRIPTOR_CONFIGURATION(2),
    DESCRIPTOR_INTERFACE_HID(sizeof(descriptor_report_generic)),
    DESCRIPTOR_INTERFACE_WEBUSB
};

uint8_t descriptor_configuration_xinput[] = {
    DESCRIPTOR_CONFIGURATION(3),
    DESCRIPTOR_INTERFACE_HID(sizeof(descriptor_report_xinput)),
    DESCRIPTOR_INTERFACE_WEBUSB,
    DESCRIPTOR_INTERFACE_XINPUT
};

uint8_t const *tud_descriptor_device_cb() {
    debug_uart("USB: tud_descriptor_device_cb\n");
    static tusb_desc_device_t descriptor_device = {DESCRIPTOR_DEVICE};
    #ifdef FW_DEVICE_ALPAKKA
        descriptor_device.idVendor = USB_TEST_VENDOR;
        descriptor_device.idProduct = USB_TEST_PRODUCT;
    #else
    if (config_get_protocol() == PROTOCOL_XINPUT_WIN) {
        descriptor_device.idVendor = USB_WIN_VENDOR;
        descriptor_device.idProduct = USB_WIN_PRODUCT;
    }
    if (config_get_protocol() == PROTOCOL_XINPUT_UNIX) {
        descriptor_device.idVendor = USB_UNIX_VENDOR;
        descriptor_device.idProduct = USB_UNIX_PRODUCT;
    }
    if (config_get_protocol() == PROTOCOL_GENERIC) {
        descriptor_device.idVendor = USB_GENERIC_VENDOR;
        descriptor_device.idProduct = USB_GENERIC_PRODUCT;
    }
    #endif
    return (uint8_t const *) &descriptor_device;
}

uint8_t const *tud_descriptor_configuration_cb(uint8_t index) {
    debug_uart("USB: tud_descriptor_configuration_cb index=0x%x\n", index);
    if (config_get_protocol() == PROTOCOL_GENERIC) {
        descriptor_configuration_generic[2] = sizeof(descriptor_configuration_generic);
        return descriptor_configuration_generic;
    } else {
        descriptor_configuration_xinput[2] = sizeof(descriptor_configuration_xinput);
        return descriptor_configuration_xinput;
    }
}

uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance) {
    debug_uart("USB: tud_hid_descriptor_report_cb\n");
    if (config_get_protocol() == PROTOCOL_GENERIC) return descriptor_report_generic;
    else return descriptor_report_xinput;
}

const uint16_t *tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    debug_uart("USB: tud_descriptor_string_cb index=0x%x\n", index);
    if (index == 0xEE && config_get_protocol() != PROTOCOL_XINPUT_UNIX) {
        static uint8_t msos[] = {MS_OS_DESCRIPTOR};
        return (uint16_t*)msos;
    }
    if (index >= sizeof(descriptor_string) / sizeof(descriptor_string[0])) {
        return NULL;
    }
    static uint16_t response[64];
    const char *string = descriptor_string[index];
    uint8_t i = 0;
    for (i; string[i]; i++) {
        response[i + 1] = string[i];
    }
    response[0] = TUSB_DESC_STRING << 8;  // String type.
    response[0] |= (i * 2) + 2;           // Total length.
    return response;
}

const bool tud_vendor_control_xfer_cb(
    uint8_t rhport,
    uint8_t stage,
    tusb_control_request_t const *request
) {
    debug_uart(
        "USB: tud_vendor_control_xfer_cb stage=%i wIndex=0x%x\n",
        stage,
        request->wIndex
    );
    if (stage != CONTROL_STAGE_SETUP) return true;
    // Compatibility IDs.
    if (
        request->wIndex == 0x0004 &&
        request->bRequest == MS_OS_VENDOR
    ) {
        if (config_get_protocol() == PROTOCOL_XINPUT_WIN) {
            static uint8_t response[] = {MS_OS_COMPATIDS_ALL};
            return tud_control_xfer(rhport, request, response, sizeof(response));
        }
        if (config_get_protocol() == PROTOCOL_GENERIC) {
            static uint8_t response[] = {MS_OS_COMPATIDS_GENERIC};
            return tud_control_xfer(rhport, request, response, sizeof(response));
        }

    }
    // Extended properties.
    if (
        request->wIndex == 0x0005 &&
        request->bRequest == MS_OS_VENDOR &&
        config_get_protocol() != PROTOCOL_XINPUT_UNIX
    ) {
        static uint8_t response[] = {MS_OS_PROPERTIES};
        return tud_control_xfer(rhport, request, response, sizeof(response));
    }
    // Return false if there is no control data to transfer.
    return false;
}

uint16_t tud_hid_get_report_cb(
    uint8_t instance,
    uint8_t report_id,
    hid_report_type_t report_type,
    uint8_t* buffer,
    uint16_t reqlen
) {
    return 0;
}

void tud_hid_set_report_cb(
    uint8_t instance,
    uint8_t report_id,
    hid_report_type_t report_type,
    uint8_t const* buffer,
    uint16_t bufsize
) {}

void tud_mount_cb(void) {
    debug_uart("USB: tud_mount_cb\n");
}

void tud_umount_cb(void) {
    debug_uart("USB: tud_umount_cb\n");
}

void tud_suspend_cb(bool remote_wakeup_en) {
    debug_uart("USB: tud_suspend_cb\n");
}

void tud_resume_cb(void) {
    debug_uart("USB: tud_resume_cb\n");
}

// Wait until the USB is able to send/receive data, until timeout
// (in milliseconds) is reached.
// Negative timeout means no timeout.
// Returns boolean depending on if it was able to send data.
bool usb_wait_for_init(int16_t timeout) {
    info("USB: Waiting for USB\n");
    led_static_mask(LED_NONE);
    led_blink_mask(LED_ALL);
    led_set_mode(LED_MODE_BLINK);
    while(timeout != 0) {
        tud_task();
        if (tud_ready()) {
            info("USB: Ready\n");
            return true;
        }
        else sleep_ms(1);
        if (timeout > 0) timeout -= 1;
    }
    warn("USB: Timeout\n");
    return false;
}

bool usb_is_connected() {
    tud_task();
    return tud_connected() && !tud_suspended();
}
