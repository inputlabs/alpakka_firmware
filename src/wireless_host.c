// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include <pico/stdlib.h>
#include <pico/time.h>
#include <pico/flash.h>
#include <hardware/watchdog.h>
#include <btstack.h>
#include "tusb_config.h"
#include "wireless.h"
#include "xinput.h"
#include "hid.h"
#include "logging.h"
#include "ctrl.h" // system clock

static host_state_t state;
static bd_addr_t peer_addr;
static uint16_t cid = 0;
static uint8_t rfcomm_server_channel;
static btstack_timer_source_t timer;
static btstack_packet_callback_registration_t hci_event_callback_registration;
static btstack_context_callback_registration_t sdp_query_callback_registration;

static void sdp_query(void * context);  // Definition.

void wireless_led_task() {
    static bool x;
    static uint32_t last = 0;
    uint32_t now = time_us_32() / 1000;
    uint16_t interval;
    if (state == CONNECTED) interval = DONGLE_LED_INTERVAL_CONNECTED;
    else interval = DONGLE_LED_INTERVAL_DISCONNECTED;
    if ((now - last) > interval) {
        x = !x;
        last = now;
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, x);
    }
}

static void secondary_loop(){
    timer.process = &secondary_loop;
    btstack_run_loop_set_timer(&timer, DONGLE_TASK_INTERVAL);
    btstack_run_loop_add_timer(&timer);
    wireless_led_task();
}

static void start_scan(void) {
    info("RF: Scanning...\n");
    state = SCANNING;
    gap_inquiry_start(DONGLE_INQUIRY_INTERVAL);
}

static void stop_scan(void) {
    info("RF: Stop scan\n");
    state = SCAN_COMPLETE;
    gap_inquiry_stop();
}

static void event_state_cb(int8_t *packet) {
    debug("RF: event_state_cb\n");
    if (btstack_event_state_get_state(packet) != HCI_STATE_WORKING) return;
    start_scan();
}

static void event_inquiry_result_cb(int8_t *packet) {
    debug("RF: event_inquiry_result_cb\n");
    if (state != SCANNING) return;
    bd_addr_t event_addr;
    uint32_t class_of_device;
    class_of_device = gap_event_inquiry_result_get_class_of_device(packet);
    gap_event_inquiry_result_get_bd_addr(packet, event_addr);
    if (class_of_device == CLASS_OF_DEVICE) {
        memcpy(peer_addr, event_addr, 6);
        info("RF: Compatible device found: %s\n", bd_addr_to_str(peer_addr));
        stop_scan();
    } else {
        info(
            "RF: Non-compatible device found: %s (0x%04x)\n",
            bd_addr_to_str(event_addr),
            (int)class_of_device
        );
    }
}

static void event_inquiry_complete_cb(int8_t *packet) {
    debug("RF: event_inquiry_complete_cb\n");
    if (state == SCANNING) {
        info("RF: Compatible device not found\n");
        start_scan();
    }
    if (state == SCAN_COMPLETE) {
        info("RF: Trying to connect\n");
        state = QUERYING;
        sdp_query_callback_registration.callback = &sdp_query;
        sdp_client_register_query_callback(&sdp_query_callback_registration);
    }
}

static void event_pin_code_request_cb(int8_t *packet) {
    debug("RF: event_pin_code_request_cb\n");
    bd_addr_t event_addr;
    hci_event_pin_code_request_get_bd_addr(packet, event_addr);
    gap_pin_code_response(event_addr, "0000");
}

static void event_channel_opened_cb(int8_t *packet) {
    debug("RF: event_channel_opened_cb\n");
    uint8_t error = rfcomm_event_channel_opened_get_status(packet);
    if (error) {
        info("RF: Channel open failed (0x%02x)\n", error);
        return;
    }
    state = CONNECTED;
    cid = rfcomm_event_channel_opened_get_rfcomm_cid(packet);
    // uint16_t rfcomm_mtu = rfcomm_event_channel_opened_get_max_frame_size(packet);
    info("RF: Connected\n");
    gap_discoverable_control(0);
    gap_connectable_control(0);
    // rfcomm_request_can_send_now_event(cid);
}

static void event_channel_closed_cb(int8_t *packet) {
    info("RF: Channel closed\n");
    cid = 0;
    start_scan();
}

// Air to queue.
void data_packet_cb(uint8_t *packet, uint16_t size) {
    // debug("RF: data_packet_cb\n");
    uint8_t index = 0;
    while (index < size) {
        uint8_t report_type = packet[index];
        index += 1;
        uint8_t entry[REPORT_QUEUE_ITEM_SIZE] = {report_type};
        if (report_type == REPORT_KEYBOARD) {
            memcpy(&entry[1], &packet[index], sizeof(KeyboardReport));
            index += sizeof(KeyboardReport);
            bool added = queue_try_add(hid_get_queue(), entry);
            // if (!added) printf("RF: Cannot add into queue\n");
            if (!added) printf("Q");
        }
        if (report_type == REPORT_MOUSE) {
            memcpy(&entry[1], &packet[index], sizeof(MouseReport) + 1);
            index += sizeof(MouseReport) + 1;
            bool added = queue_try_add(hid_get_queue(), entry);
            // if (!added) printf("RF: Cannot add into queue\n");
            if (!added) printf("Q");
        }
        if (report_type == REPORT_MOUSE_EOT) {
            bool added = queue_try_add(hid_get_queue(), entry);
            if (!added) printf("Q");
        }
        if (report_type == REPORT_XINPUT) {
            memcpy(&entry[1], &packet[index], sizeof(XInputReport));
            index += sizeof(XInputReport);
            bool added = queue_try_add(hid_get_queue(), entry);
            // if (!added) printf("RF: Cannot add into queue\n");
            if (!added) printf("Q");
        }
    }
}

static void event_handler(uint8_t *packet) {
    debug("RF: event_handler\n");
    uint8_t event_type = hci_event_packet_get_type(packet);
    if (event_type == BTSTACK_EVENT_STATE) event_state_cb(packet);
    if (event_type == GAP_EVENT_INQUIRY_RESULT) event_inquiry_result_cb(packet);
    if (event_type == GAP_EVENT_INQUIRY_COMPLETE) event_inquiry_complete_cb(packet);
    if (event_type == HCI_EVENT_PIN_CODE_REQUEST) event_pin_code_request_cb(packet);
    if (event_type == RFCOMM_EVENT_CHANNEL_OPENED) event_channel_opened_cb(packet);
    if (event_type == RFCOMM_EVENT_CHANNEL_CLOSED) event_channel_closed_cb(packet);
}

static void packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
    // debug("RF: packet_handler\n");
    if (packet_type == HCI_EVENT_PACKET) event_handler(packet);
    if (packet_type == RFCOMM_DATA_PACKET) data_packet_cb(packet, size);
}

static void sdp_query_hander(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
    debug("RF: sdp_query_hander\n");
    uint8_t event_type = hci_event_packet_get_type(packet);
    if (event_type == SDP_EVENT_QUERY_RFCOMM_SERVICE) {
        rfcomm_server_channel = sdp_event_query_rfcomm_service_get_rfcomm_channel(packet);
    }
    if (event_type == SDP_EVENT_QUERY_COMPLETE) {
        uint8_t status = sdp_event_query_complete_get_status(packet);
        if (status) {
            info("RF: SDP query failed, status 0x%02x\n", sdp_event_query_complete_get_status(packet));
            return;
        }
        if (rfcomm_server_channel) {
            info("RF: SDP query done, channel %i\n", rfcomm_server_channel);
            rfcomm_create_channel(packet_handler, peer_addr, rfcomm_server_channel, NULL);
        } else {
            info("RF: No SPP service found\n");
        }
    }
}

static void sdp_query(void *context) {
    debug("RF: sdp_query\n");
    if (state != QUERYING) return;
    state = CONNECTING;
    sdp_client_query_rfcomm_channel_and_name_for_uuid(
        &sdp_query_hander,
        peer_addr,
        BLUETOOTH_ATTRIBUTE_PUBLIC_BROWSE_ROOT
    );
}

void wireless_host_init() {
    info("RF: Host init (core %i)\n", get_core_num());
    flash_safe_execute_core_init();
    cyw43_arch_init();
    cyw43_pm_value(CYW43_NO_POWERSAVE_MODE, POWER_MANAGEMENT_SLEEP_TIMEOUT, 1, 1, 1);
    l2cap_init();
    rfcomm_init();
    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);
    gap_ssp_set_io_capability(SSP_IO_CAPABILITY_DISPLAY_YES_NO); // ???
	hci_power_control(HCI_POWER_ON);
    info("RF: Host loop\n");
    secondary_loop();
    btstack_run_loop_execute();
}
