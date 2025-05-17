// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#pragma once
#include <stdint.h>
#include <stdbool.h>

#define BOARD_DEVICE_RHPORT_NUM 0
#define BOARD_DEVICE_RHPORT_SPEED  OPT_MODE_FULL_SPEED

// #define CFG_TUSB_DEBUG 2
#define CFG_TUSB_RHPORT0_MODE (OPT_MODE_DEVICE | BOARD_DEVICE_RHPORT_SPEED)
#define CFG_TUSB_MEM_SECTION
#define CFG_TUSB_MEM_ALIGN __attribute__ ((aligned(4)))
#define CFG_TUD_ENDPOINT0_SIZE 64
#define CFG_TUD_VENDOR_RX_BUFSIZE 64
#define CFG_TUD_VENDOR_TX_BUFSIZE 64

#define CFG_TUD_HID 1
#define CFG_TUD_CDC 0
#define CFG_TUD_MSC 0
#define CFG_TUD_MIDI 0
#define CFG_TUD_VENDOR 1

#define ITF_HID 0
#define ITF_WEBUSB 1
#define ITF_XINPUT 2

#define ADDR_HID_IN 0x86
#define ADDR_WEBUSB_IN 0x83
#define ADDR_WEBUSB_OUT 0x04
#define ADDR_XINPUT_IN 0x81
#define ADDR_XINPUT_OUT 0x02

#define STRING_VENDOR "Input Labs"

#define STRING_PRODUCT_ALPAKKA "Alpakka"
#define STRING_PRODUCT_DONGLE "Dongle"

#define STRING_VERSION_ALPAKKA_V0 "v0"
#define STRING_VERSION_ALPAKKA_V1 "v1"
#define STRING_VERSION_DONGLE_V1 "v1"

#define STRING_HID "HID"
#define STRING_WEBUSB "WEBUSB"
#define STRING_XINPUT "XINPUT_GENERIC_CONTROLLER"

#define MS_OS_VENDOR 0x17

#define USB_WIN_VENDOR  0x0170  // Input Labs.
#define USB_WIN_PRODUCT_ALPAKKA 0xAA80  // Alpakka (Xinput)
#define USB_WIN_PRODUCT_DONGLE  0xDA80  // Dongle (Xinput)

#define USB_UNIX_VENDOR  0x045E  // 360 controller vendor.
#define USB_UNIX_PRODUCT 0x028E  // 360 controller product.

#define USB_GENERIC_VENDOR  0x0170  // Input Labs.
#define USB_GENERIC_PRODUCT_ALPAKKA 0xAC80  // Alpakka (HID complilant gamepad)
#define USB_GENERIC_PRODUCT_DONGLE  0xDC80  // Dongle (HID complilant gamepad)

#define USB_TEST_VENDOR  0x0170  // Input Labs.
#define USB_TEST_PRODUCT 0xFF00  // Test.

#ifdef DEVICE_IS_ALPAKKA
    #define WEBUSB_ID  'A', 0, '0', 0, '0', 0, '8', 0, '0', 0
#elif defined DEVICE_DONGLE
    #define WEBUSB_ID  'D', 0, '0', 0, '0', 0, '8', 0, '0', 0
#else
    #define WEBUSB_ID  'X', 0, '0', 0, '0', 0, '8', 0, '0', 0
#endif

#define DESCRIPTOR_DEVICE \
    0x12,    /* .bLength */\
    0x01,    /* .bDescriptorType */\
    0x0200,  /* .bcdUSB */\
    0x00,    /* .bDeviceClass */\
    0x00,    /* .bDeviceSubClass */\
    0x00,    /* .bDeviceProtocol */\
    0x40,    /* .bMaxPacketSize0 */\
    0x0000,  /* .idVendor */\
    0x0000,  /* .idProduct */\
    0x0114,  /* .bcdDevice */\
    0x01,    /* .iManufacturer */\
    0x02,    /* .iProduct */\
    0x03,    /* .iSerialNumber */\
    0x01     /* .bNumConfiguration */

#define DESCRIPTOR_CONFIGURATION(itfs) \
    0x09,        /* bLength */\
    0x02,        /* bDescriptorType: configuration */\
    0x00, 0x00,  /* wTotalLength */\
    itfs,        /* bNumInterfaces */\
    0x01,        /* bConfigurationValue */\
    0x00,        /* iConfiguration */\
    0x80,        /* bmAttributes */\
    0xFA         /* bMaxPower */

#define DESCRIPTOR_INTERFACE_HID(report_size) \
    TUD_HID_DESCRIPTOR( \
        ITF_HID,                /* Interface index */\
        ITF_HID + 4,            /* String index */\
        HID_ITF_PROTOCOL_NONE,  /* Boot protocol */\
        report_size,            /* Report descriptor length */\
        ADDR_HID_IN,            /* Interface address */\
        32,                     /* Endpoint buffer size */\
        1                       /* Interface interval (ms) */\
    )

#define DESCRIPTOR_INTERFACE_WEBUSB \
    TUD_VENDOR_DESCRIPTOR( \
        ITF_WEBUSB,       /* Interface index */\
        ITF_WEBUSB + 4,   /* String index */\
        ADDR_WEBUSB_OUT,  /* Address out */\
        ADDR_WEBUSB_IN,   /* Address in */\
        64                /* Size */\
    )

#define DESCRIPTOR_INTERFACE_XINPUT \
    0x09,        /* bLength */\
    0x04,        /* bDescriptorType: interface */\
    ITF_XINPUT,  /* bInterfaceNumber */\
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
    0x07,            /* bLength */\
    0x05,            /* bDescriptorType: endpoint */\
    ADDR_XINPUT_IN,  /* bEndpointAddress */\
    0x03,            /* bmAttributes */\
    0x20, 0x00,      /* wMaxPacketSize */\
    0x04             /* bInterval */\

#define DESCRIPTOR_ENDPOINT_XINPUT_OUT \
    0x07,             /* bLength */\
    0x05,             /* bDescriptorType: endpoint */\
    ADDR_XINPUT_OUT,  /* bEndpointAddress */\
    0x03,             /* bmAttributes */\
    0x20, 0x00,       /* wMaxPacketSize */\
    0x08              /* bInterval */

#define MS_OS_DESCRIPTOR \
    0x12,                    /* Length */\
    0x03,                    /* Descriptor type: string */\
    'M' , 0x00, 'S' , 0x00,  /* Signature */\
    'F' , 0x00, 'T' , 0x00,  \
    '1' , 0x00, '0' , 0x00,  \
    '0' , 0x00,              \
    MS_OS_VENDOR,            /* Vendor code */\
    0x00                     /* Padding */

#define MS_OS_COMPATIDS(len, itfs) \
    len , 0x00, 0x00, 0x00,  /* Length */\
    0x00, 0x01,              /* Version */\
    0x04, 0x00,              /* Descriptor type: Compatibility ID */\
    itfs, 0x00,              /* Sections */\
    0x00, 0x00, 0x00, 0x00,  /* Reserved */\
    0x00, 0x00               /* Reserved */\

#define MS_OS_COMPATIDS_WINUSB \
    ITF_WEBUSB,              /* Interface index */\
    0x01,                    /* Reserved */\
    'W' , 'I' , 'N' , 'U' ,  /* Compat ID */\
    'S' , 'B' , 0x00, 0x00,  \
    0x00, 0x00, 0x00, 0x00,  /* Sub-compat ID. */\
    0x00, 0x00, 0x00, 0x00,  \
    0x00, 0x00, 0x00, 0x00,  /* Reserved */\
    0x00, 0x00               /* Reserved */\

#define MS_OS_COMPATIDS_XUSB \
    ITF_XINPUT,              /* Interface index */\
    0x01,                    /* Reserved */\
    'X' , 'U' , 'S' , 'B',   /* Compat ID */\
    '1' , '0' , 0x00, 0x00,  \
    0x00, 0x00, 0x00, 0x00,  /* Sub-compat ID. */\
    0x00, 0x00, 0x00, 0x00,  \
    0x00, 0x00, 0x00, 0x00,  /* Reserved */\
    0x00, 0x00               /* Reserved */

#define MS_OS_COMPATIDS_ALL \
    MS_OS_COMPATIDS(64, 2), \
    MS_OS_COMPATIDS_WINUSB, \
    MS_OS_COMPATIDS_XUSB

#define MS_OS_COMPATIDS_GENERIC \
    MS_OS_COMPATIDS(40, 1), \
    MS_OS_COMPATIDS_WINUSB

#define MS_OS_PROPERTIES \
    0x8E, 0x00, 0x00, 0x00,  /* Length */\
    0x00, 0x01,              /* Version */\
    0x05, 0x00,              /* Descriptor type: Extented property*/\
    0x01, 0x00,              /* Sections */\
    0x84, 0x00, 0x00, 0x00,  /* Section size */\
    0x01, 0x00, 0x00, 0x00,  /* Data type */\
    0x28, 0x00,              /* Property name size */\
    /* Property name */\
    'D', 0, 'e', 0, 'v', 0, 'i', 0, 'c', 0, 'e', 0, 'I', 0, 'n', 0, \
    't', 0, 'e', 0, 'r', 0, 'f', 0, 'a', 0, 'c', 0, 'e', 0, 'G', 0, \
    'U', 0, 'I', 0, 'D', 0,  0 , 0, \
    /* Data size */\
    0x4E, 0x00, 0x00, 0x00, \
    /* Data */\
    '{', 0, 'A', 0, '0', 0, '0', 0, '6', 0, 'C', 0, '7', 0, 'D', 0, \
    'A', 0, '-', 0, '8', 0, '8', 0, 'A', 0, '4', 0, '-', 0, '4', 0, \
    '7', 0, 'E', 0, 'E', 0, '-', 0, 'A', 0, '7', 0, '5', 0, '2', 0, \
    '-', 0, 'F', 0, 'B', 0, 'C', 0, '4', 0, '2', 0, '2', 0, '5', 0, \
    WEBUSB_ID, \
    '}', 0,  0,  0

// Mouse HID definition that differs from the default implementation included
// in TinyUSB. (Custom 16bit deltas).
// https://github.com/hathach/tinyusb/blob/ae364b1460b91153cd94b4b0303eeda6419ff1d1/src/class/hid/hid_device.h#L221
#define TUD_HID_REPORT_DESC_MOUSE_CUSTOM(...) \
  HID_USAGE_PAGE ( HID_USAGE_PAGE_DESKTOP      )                   ,\
  HID_USAGE      ( HID_USAGE_DESKTOP_MOUSE     )                   ,\
  HID_COLLECTION ( HID_COLLECTION_APPLICATION  )                   ,\
    /* Report ID if any */\
    __VA_ARGS__ \
    HID_USAGE      ( HID_USAGE_DESKTOP_POINTER )                   ,\
    HID_COLLECTION ( HID_COLLECTION_PHYSICAL   )                   ,\
      HID_USAGE_PAGE  ( HID_USAGE_PAGE_BUTTON  )                   ,\
        HID_USAGE_MIN   ( 1                                      ) ,\
        HID_USAGE_MAX   ( 5                                      ) ,\
        HID_LOGICAL_MIN ( 0                                      ) ,\
        HID_LOGICAL_MAX ( 1                                      ) ,\
        /* Left, Right, Middle, Backward, Forward buttons */ \
        HID_REPORT_COUNT( 5                                      ) ,\
        HID_REPORT_SIZE ( 1                                      ) ,\
        HID_INPUT       ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ) ,\
        /* 3 bit padding */ \
        HID_REPORT_COUNT( 1                                      ) ,\
        HID_REPORT_SIZE ( 3                                      ) ,\
        HID_INPUT       ( HID_CONSTANT                           ) ,\
      HID_USAGE_PAGE  ( HID_USAGE_PAGE_DESKTOP )                   ,\
        /* X, Y position [-32767, 32767] */ \
        HID_USAGE       ( HID_USAGE_DESKTOP_X                    ) ,\
        HID_USAGE       ( HID_USAGE_DESKTOP_Y                    ) ,\
        HID_LOGICAL_MIN_N ( 0x8000, 2                            ) /* CHANGED */ ,\
        HID_LOGICAL_MAX_N ( 0x7FFF, 2                            ) /* CHANGED */ ,\
        HID_REPORT_COUNT( 2                                      ) /* CHANGED */ ,\
        HID_REPORT_SIZE ( 16                                     ) /* CHANGED */ ,\
        HID_INPUT       ( HID_DATA | HID_VARIABLE | HID_RELATIVE ) ,\
        /* Vertical wheel scroll [-127, 127] */ \
        HID_USAGE       ( HID_USAGE_DESKTOP_WHEEL                ) ,\
        HID_LOGICAL_MIN ( 0x81                                   ) ,\
        HID_LOGICAL_MAX ( 0x7f                                   ) ,\
        HID_REPORT_COUNT( 1                                      ) ,\
        HID_REPORT_SIZE ( 8                                      ) ,\
        HID_INPUT       ( HID_DATA | HID_VARIABLE | HID_RELATIVE ) ,\
      HID_USAGE_PAGE  ( HID_USAGE_PAGE_CONSUMER ), \
        /* Horizontal wheel scroll [-127, 127] */ \
        HID_USAGE_N     ( HID_USAGE_CONSUMER_AC_PAN, 2           ), \
        HID_LOGICAL_MIN ( 0x81                                   ), \
        HID_LOGICAL_MAX ( 0x7f                                   ), \
        HID_REPORT_COUNT( 1                                      ), \
        HID_REPORT_SIZE ( 8                                      ), \
        HID_INPUT       ( HID_DATA | HID_VARIABLE | HID_RELATIVE ), \
    HID_COLLECTION_END                                            , \
  HID_COLLECTION_END \

// Gamepad HID definition that differs from the default implementation included
// in TinyUSB. (16bit axis, different button layout).
// https://github.com/hathach/tinyusb/blob/7bf5923052e5861f54c9cb0581e328f8be26a0a9/src/class/hid/hid_device.h#L315
#define TUD_HID_REPORT_DESC_GAMEPAD_CUSTOM(...) \
  HID_USAGE_PAGE ( HID_USAGE_PAGE_DESKTOP     )                  ,\
  HID_USAGE      ( HID_USAGE_DESKTOP_GAMEPAD  )                  ,\
  HID_COLLECTION ( HID_COLLECTION_APPLICATION )                  ,\
    /* Report ID */ \
    __VA_ARGS__ \
    /* Axis */ \
    HID_USAGE_PAGE    ( HID_USAGE_PAGE_DESKTOP                 ) ,\
    HID_USAGE         ( HID_USAGE_DESKTOP_X                    ) ,\
    HID_USAGE         ( HID_USAGE_DESKTOP_Y                    ) ,\
    HID_USAGE         ( HID_USAGE_DESKTOP_RX                   ) ,\
    HID_USAGE         ( HID_USAGE_DESKTOP_RY                   ) ,\
    HID_USAGE         ( HID_USAGE_DESKTOP_Z                    ) ,\
    HID_USAGE         ( HID_USAGE_DESKTOP_RZ                   ) ,\
    HID_LOGICAL_MIN_N ( -32767, 2                              ) /* CHANGED */ ,\
    HID_LOGICAL_MAX_N ( 32767, 2                               ) /* CHANGED */ ,\
    HID_REPORT_COUNT  ( 6                                      ) /* CHANGED */ ,\
    HID_REPORT_SIZE   ( 16                                     ) /* CHANGED */ ,\
    HID_INPUT         ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ) ,\
    /* Button mask */ \
    HID_USAGE_PAGE    ( HID_USAGE_PAGE_BUTTON                  ) ,\
    HID_USAGE_MIN     ( 1                                      ) ,\
    HID_USAGE_MAX     ( 32                                     ) ,\
    HID_LOGICAL_MIN   ( 0                                      ) ,\
    HID_LOGICAL_MAX   ( 1                                      ) ,\
    HID_REPORT_COUNT  ( 32                                     ) ,\
    HID_REPORT_SIZE   ( 1                                      ) ,\
    HID_INPUT         ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ) ,\
  HID_COLLECTION_END

bool usb_wait_for_init(int16_t timeout);
bool usb_is_connected();
