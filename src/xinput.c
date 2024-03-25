// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <tusb.h>
#include <device/usbd_pvt.h>
#include "xinput.h"
#include "tusb_config.h"
#include "logging.h"

const uint8_t ep_in[] = {DESCRIPTOR_ENDPOINT_XINPUT_IN};
const uint8_t ep_out[] = {DESCRIPTOR_ENDPOINT_XINPUT_OUT};

static void xinput_init(void) {}

static void xinput_reset(uint8_t rhport) {}

static uint16_t xinput_open(
    uint8_t rhport,
    tusb_desc_interface_t const *itf_desc,
    uint16_t max_len
) {
    debug_uart(
        "USB: xinput_open rhport=%i itf=0x%x max_len=%i\n",
        rhport,
        itf_desc->iInterface,
        max_len
    );
    if (itf_desc->iInterface == 0) {
        usbd_edpt_open(rhport, (tusb_desc_endpoint_t const *)ep_in);
        usbd_edpt_open(rhport, (tusb_desc_endpoint_t const *)ep_out);
        return (
            sizeof(tusb_desc_interface_t) +
            16 +
            (sizeof(tusb_desc_endpoint_t) * 2)
        );
    }
    return 0;
}

static bool xinput_control_xfer_cb(
    uint8_t rhport,
    uint8_t stage,
    tusb_control_request_t const *request
) {
    // printf("xinput_control_xfer_cb\n");
    return true;
}

static bool xinput_xfer_cb(
    uint8_t rhport,
    uint8_t ep_addr,
    xfer_result_t result,
    uint32_t xferred_bytes
) {
    // printf("xinput_xfer_cb\n");
    return true;
}

static usbd_class_driver_t const xinput_driver = {
    .init            = xinput_init,
    .reset           = xinput_reset,
    .open            = xinput_open,
    .control_xfer_cb = xinput_control_xfer_cb,
    .xfer_cb         = xinput_xfer_cb,
    .sof             = NULL
};

usbd_class_driver_t const *usbd_app_driver_get_cb(uint8_t *driver_count) {
    *driver_count = 1;
    return &xinput_driver;
}

bool xinput_send_report(XInputReport *report) {
    bool result = false;
    if (!usbd_edpt_busy(0, ADDR_XINPUT_IN)) {
        usbd_edpt_claim(0, ADDR_XINPUT_IN);
        result = usbd_edpt_xfer(0, ADDR_XINPUT_IN, (uint8_t*)report, XINPUT_REPORT_SIZE);
        usbd_edpt_release(0, ADDR_XINPUT_IN);
    }
    return result;
}

// void xinput_receive_report() {
//     uint8_t addr = ((tusb_desc_endpoint_t *)ep_out)->bEndpointAddress;
//     uint8_t buffer[8] = {0, 0, 0, 0, 0, 0, 0, 0};
//     usbd_edpt_claim(0, addr);
//     usbd_edpt_xfer(0, addr, buffer, 8);
//     usbd_edpt_release(0, addr);
//     printf(
//         "BUF %i %i %i %i %i %i %i %i\n",
//         buffer[0],
//         buffer[1],
//         buffer[2],
//         buffer[3],
//         buffer[4],
//         buffer[5],
//         buffer[6],
//         buffer[7]
//     );
// }
