// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#pragma once

#define BOARD_DEVICE_RHPORT_NUM 0
#define BOARD_DEVICE_RHPORT_SPEED  OPT_MODE_FULL_SPEED

// #define CFG_TUSB_DEBUG 2
#define CFG_TUSB_RHPORT0_MODE (OPT_MODE_DEVICE | BOARD_DEVICE_RHPORT_SPEED)
#define CFG_TUSB_MEM_SECTION
#define CFG_TUSB_MEM_ALIGN __attribute__ ((aligned(4)))
#define CFG_TUD_ENDPOINT0_SIZE 64

#define CFG_TUD_HID 1
#define CFG_TUD_CDC 0
#define CFG_TUD_MSC 0
#define CFG_TUD_MIDI 0
#define CFG_TUD_VENDOR 0

#define REPORT_KEYBOARD 1
#define REPORT_MOUSE 2

#define STRING_VENDOR "ILO"
#define STRING_PRODUCT "Device_A"
#define STRING_DEVICE_VERSION "1.0"
#define STRING_INTERFACE_0 "HID"
#define STRING_INTERFACE_1 "XINPUT_GENERIC_CONTROLLER"

#define WCID_VENDOR 0x17

#define OS_WIN_VENDOR_ID 0x0170  // Input Labs.
#define OS_WIN_PRODUCT_ID 0xA090
#define OS_UNIX_VENDOR_ID 0x045E   // Xbox controller vendor.
#define OS_UNIX_PRODUCT_ID 0x028E  // Xbox controller product.


#define DESCRIPTOR_DEVICE \
    0x12,        /* .bLength */\
    0x01,        /* .bDescriptorType */\
    0x0200,      /* .bcdUSB */\
    0x00,        /* .bDeviceClass */\
    0x00,        /* .bDeviceSubClass */\
    0x00,        /* .bDeviceProtocol */\
    0x40,        /* .bMaxPacketSize0 */\
    0x0000,      /* .idVendor */\
    0x0000,      /* .idProduct */\
    0x0114,      /* .bcdDevice */\
    0x01,        /* .iManufacturer */\
    0x02,        /* .iProduct */\
    0x03,        /* .iSerialNumber */\
    0x01         /* .bNumConfiguration */

#define DESCRIPTOR_CONFIGURATION \
    0x09,        /* bLength */\
    0x02,        /* bDescriptorType: configuration */\
    0x00, 0x00,  /* wTotalLength */\
    0x02,        /* bNumInterfaces */\
    0x01,        /* bConfigurationValue */\
    0x00,        /* iConfiguration */\
    0x80,        /* bmAttributes */\
    0xFA         /* bMaxPower */

#define DESCRIPTOR_INTERFACE_XINPUT \
    0x09,        /* bLength */\
    0x04,        /* bDescriptorType: interface */\
    0x01,        /* bInterfaceNumber */\
    0x00,        /* bAlternateSetting */\
    0x02,        /* bNumEndpoints */\
    0xFF,        /* bInterfaceClass */\
    0x5D,        /* bInterfaceSubClass */\
    0x01,        /* bInterfaceProtocol */\
    0x00,        /* iInterface */\
    /* Undocumented */\
    0x10, 0x21, 0x10, 0x01, 0x01, 0x24, 0x81, 0x14, \
    0x03, 0x00, 0x03, 0x13, 0x02, 0x00, 0x03, 0x00, \
    /* Endpoints */\
    DESCRIPTOR_ENDPOINT_XINPUT_IN, \
    DESCRIPTOR_ENDPOINT_XINPUT_OUT

#define DESCRIPTOR_ENDPOINT_XINPUT_IN \
    0x07,        /* bLength */\
    0x05,        /* bDescriptorType: endpoint */\
    0x81,        /* bEndpointAddress */\
    0x03,        /* bmAttributes */\
    0x20, 0x00,  /* wMaxPacketSize */\
    0x04         /* bInterval */\

#define DESCRIPTOR_ENDPOINT_XINPUT_OUT \
    0x07,        /* bLength */\
    0x05,        /* bDescriptorType: endpoint */\
    0x02,        /* bEndpointAddress */\
    0x03,        /* bmAttributes */\
    0x20, 0x00,  /* wMaxPacketSize */\
    0x08         /* bInterval */

#define DESCRIPTOR_INTERFACE_HID(report_size) \
    TUD_HID_DESCRIPTOR( \
        0,                      /* Interface index */\
        4,                      /* String index */\
        HID_ITF_PROTOCOL_NONE,  /* Boot protocol */\
        report_size,            /* Report descriptor length */\
        0x86,                   /* Interface address */\
        32,                     /* Endpoint buffer size */\
        1                       /* Interface interval (ms) */\
    )

#define MS_OS_DESCRIPTORS_MAGIC_PAYLOAD \
    0x12,                    /* Length */\
    0x03,                    /* Descriptor type: string */\
    0x4D, 0x00, 0x53, 0x00,  /* MSFT100 */\
    0x46, 0x00, 0x54, 0x00,  /* ... */\
    0x31, 0x00, 0x30, 0x00,  /* ... */\
    0x30, 0x00,              /* ... */\
    WCID_VENDOR,             /* Vendor code */\
    0x00                     /* Padding */

#define MS_WCID_MAGIC_PAYLOAD \
    0x28, 0x00, 0x00, 0x00,  /* Length */\
    0x00, 0x01,              /* Version */\
    0x04, 0x00,              /* Descriptor type: Compatibility ID */\
    0x01,                    /* Sections */\
    0x00, 0x00, 0x00, 0x00,  /* Reserved */\
    0x00, 0x00, 0x00,        /* Reserved */\
    0x01,                    /* Xinput interface index */\
    0x01,                    /* Reserved */\
    0x58, 0x55, 0x53, 0x42,  /* Compat ID: XUSB10__ */\
    0x31, 0x30, 0x00, 0x00,  /* ... */\
    0x00, 0x00, 0x00, 0x00,  /* Sub-compat ID. */\
    0x00, 0x00, 0x00, 0x00,  /* ... */\
    0x00, 0x00, 0x00, 0x00,  /* Reserved */\
    0x00, 0x00               /* Reserved */
