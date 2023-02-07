// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <tusb_config.h>
#include <tusb.h>
#include "config.h"

static const char *const descriptor_string[] = {
    (const char[]){0x09, 0x04},
    STRING_VENDOR,
    STRING_PRODUCT,
    STRING_DEVICE_VERSION,
    STRING_INTERFACE_0,
    STRING_INTERFACE_1,
};

uint8_t const descriptor_report_generic[] = {
    TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(REPORT_KEYBOARD)),
    TUD_HID_REPORT_DESC_MOUSE(HID_REPORT_ID(REPORT_MOUSE)),
    TUD_HID_REPORT_DESC_GAMEPAD(HID_REPORT_ID(REPORT_GAMEPAD)),
};

uint8_t const descriptor_report_xinput[] = {
    TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(REPORT_KEYBOARD)),
    TUD_HID_REPORT_DESC_MOUSE(HID_REPORT_ID(REPORT_MOUSE)),
};

uint8_t descriptor_configuration_generic[] = {
    DESCRIPTOR_CONFIGURATION(0x01),
    DESCRIPTOR_INTERFACE_HID(sizeof(descriptor_report_generic))
};

uint8_t descriptor_configuration_xinput[] = {
    DESCRIPTOR_CONFIGURATION(0x02),
    DESCRIPTOR_INTERFACE_HID(sizeof(descriptor_report_xinput)),
    DESCRIPTOR_INTERFACE_XINPUT
};

uint8_t const *tud_descriptor_device_cb() {
    printf("USB: tud_descriptor_device_cb\n");
    static tusb_desc_device_t descriptor_device = {DESCRIPTOR_DEVICE};
    if (config_get_os_mode() == OS_MODE_XINPUT_WIN) {
        descriptor_device.idVendor = USB_WIN_VENDOR;
        descriptor_device.idProduct = USB_WIN_PRODUCT;
    }
    if (config_get_os_mode() == OS_MODE_XINPUT_UNIX) {
        descriptor_device.idVendor = USB_UNIX_VENDOR;
        descriptor_device.idProduct = USB_UNIX_PRODUCT;
    }
    if (config_get_os_mode() == OS_MODE_GENERIC) {
        descriptor_device.idVendor = USB_GENERIC_VENDOR;
        descriptor_device.idProduct = USB_GENERIC_PRODUCT;
    }
    return (uint8_t const *) &descriptor_device;
}

uint8_t const *tud_descriptor_configuration_cb(uint8_t index) {
    printf("USB: tud_descriptor_configuration_cb index=0x%x\n", index);
    descriptor_configuration_generic[2] = sizeof(descriptor_configuration_generic);
    descriptor_configuration_xinput[2] = sizeof(descriptor_configuration_xinput);
    if (config_get_os_mode() == OS_MODE_GENERIC) return descriptor_configuration_generic;
    else return descriptor_configuration_xinput;
}

uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance) {
    printf("USB: tud_hid_descriptor_report_cb\n");
    if (config_get_os_mode() == OS_MODE_GENERIC) return descriptor_report_generic;
    else return descriptor_report_xinput;
}

const uint16_t *tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    printf("USB: tud_descriptor_string_cb index=0x%x\n", index);
    if (index == 0xEE && config_get_os_mode() == OS_MODE_XINPUT_WIN) {
        static uint8_t msos[18] = {MS_OS_DESCRIPTORS_MAGIC_PAYLOAD};
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
    printf(
        "USB: tud_vendor_control_xfer_cb stage=%i wIndex=0x%x\n",
        stage,
        request->wIndex
    );
    if (stage != CONTROL_STAGE_SETUP) return true;
    if (
        request->wIndex == 0x0004 &&
        request->bRequest == WCID_VENDOR &&
        config_get_os_mode() == OS_MODE_XINPUT_WIN)
    {
        static uint8_t response[40] = {MS_WCID_MAGIC_PAYLOAD};
        return tud_control_xfer(rhport, request, response, 40);
    }
    return true;
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
