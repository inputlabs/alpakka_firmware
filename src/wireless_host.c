// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include <pico/stdlib.h>
#include <pico/time.h>
#include <hardware/watchdog.h>
#include <btstack.h>
#include "tusb_config.h"
#include "wireless.h"
#include "hid.h"
#include "logging.h"
#include "ctrl.h" // system clock

static host_state_t state;
static bd_addr_t peer_addr;
static uint16_t cid = 0;
static uint8_t rfcomm_server_channel;
static btstack_packet_callback_registration_t hci_event_callback_registration;
static btstack_context_callback_registration_t sdp_query_callback_registration;

static void sdp_query(void * context);  // Definition.

void wireless_led_task() {
    static bool x;
    static uint32_t last = 0;
    uint32_t now = time_us_32() / 1000;
    uint16_t interval;
    if (state == CONNECTED) interval = 500;
    else interval = 100;
    if ((now - last) > interval) {
        x = !x;
        last = now;
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, x);
    }
}

uint16_t period = 4;  // ms
static btstack_timer_source_t loop_timer;

static void loop_task(btstack_timer_source_t *ts){
    btstack_run_loop_set_timer(ts, period);
    btstack_run_loop_add_timer(ts);
    wireless_led_task();
    host_task();
}

static void loop_setup(void){
    info("WL: Device loop setup\n");
    loop_timer.process = &loop_task;
    btstack_run_loop_set_timer(&loop_timer, period);
    btstack_run_loop_add_timer(&loop_timer);
}

void wireless_process_packet(uint8_t *packet, uint16_t size) {
    uint32_t received = time_us_32();
    static uint32_t last = 0;
    static uint32_t last_print = 0;
    static uint16_t num = 0;
    static uint16_t max = 0;
    uint16_t elapsed = (received-last) / 1000;
    if (elapsed > max) max = elapsed;
    num += 1;
    last = time_us_32();
    if(elapsed > 6) printf("%i ", elapsed);
    if(time_us_32()-last_print > 1000000) {
        last_print = time_us_32();
        info("num=%i max=%i\n", num, max);
        num = max = 0;
    }
    uint8_t report_type = packet[0];
    if (report_type == REPORT_KEYBOARD) {
        uint8_t modifiers = packet[1];
        uint8_t keys[6];
        memcpy(keys, &packet[3], 6);
        hid_report_direct_keyboard(modifiers, keys);
    }
    if (report_type == REPORT_MOUSE) {
        uint8_t buttons = packet[1];
        int16_t x = (packet[2] << 8) + packet[3];
        int16_t y = (packet[4] << 8) + packet[5];
        // int16_t scroll = report[7];
        hid_report_direct_mouse(buttons, x, y, 0);
    }
}

static void start_scan(void) {
    printf("WL: Scanning...\n");
    state = SCANNING;
    gap_inquiry_start(INQUIRY_INTERVAL);
}

static void stop_scan(void) {
    state = SCAN_COMPLETE;
    gap_inquiry_stop();
}

static void event_handler(uint8_t *packet, uint16_t size) {
    bd_addr_t event_addr;
    uint8_t rfcomm_channel;
    uint32_t class_of_device;
    uint8_t event_type = hci_event_packet_get_type(packet);
    if (event_type == BTSTACK_EVENT_STATE) {
        if (btstack_event_state_get_state(packet) != HCI_STATE_WORKING) return;
        start_scan();
    }
    if (event_type == GAP_EVENT_INQUIRY_RESULT) {
        if (state != SCANNING) return;
        class_of_device = gap_event_inquiry_result_get_class_of_device(packet);
        gap_event_inquiry_result_get_bd_addr(packet, event_addr);
        if (class_of_device == CLASS_OF_DEVICE) {
            memcpy(peer_addr, event_addr, 6);
            printf("WL: Compatible device found: %s\n", bd_addr_to_str(peer_addr));
            stop_scan();
        } else {
            printf("WL: Device found: %s (0x%04x)\n", bd_addr_to_str(event_addr), (int)class_of_device);
        }
    }
    if (event_type == GAP_EVENT_INQUIRY_COMPLETE) {
        if (state == SCANNING) {
            printf("WL: Compatible device not found\n");
            start_scan();
        }
        if (state == SCAN_COMPLETE) {
            printf("WL: Trying to connect\n");
            state = QUERYING;
            sdp_query_callback_registration.callback = &sdp_query;
            sdp_client_register_query_callback(&sdp_query_callback_registration);
        }
    }
    if (event_type == HCI_EVENT_PIN_CODE_REQUEST) {
        hci_event_pin_code_request_get_bd_addr(packet, event_addr);
        gap_pin_code_response(event_addr, "0000");
    }
    if (event_type == RFCOMM_EVENT_CHANNEL_OPENED) {
            uint8_t error = rfcomm_event_channel_opened_get_status(packet);
        if (!error) {
            state = CONNECTED;
            cid = rfcomm_event_channel_opened_get_rfcomm_cid(packet);
            // uint16_t rfcomm_mtu = rfcomm_event_channel_opened_get_max_frame_size(packet);
            printf("WL: Channel open succeeded\n");
            gap_discoverable_control(0);
            gap_connectable_control(0);
            rfcomm_request_can_send_now_event(cid);
        } else {
            printf("WL: Channel open failed (0x%02x)\n", error);
        }
    }
    if (event_type == RFCOMM_EVENT_CHANNEL_CLOSED) {
        printf("WL: Channel closed\n");
        cid = 0;
        start_scan();
    }
}

static void packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
    if (packet_type == HCI_EVENT_PACKET) {
        event_handler(packet, size);
    }
    if (packet_type == RFCOMM_DATA_PACKET) {
        uint8_t shift = 1;  // TODO: Better way to unwrap?
        wireless_process_packet(&packet[shift], size-shift);
    }
}

static void sdp_query_hander(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
    uint8_t event_type = hci_event_packet_get_type(packet);
    if (event_type == SDP_EVENT_QUERY_RFCOMM_SERVICE) {
        rfcomm_server_channel = sdp_event_query_rfcomm_service_get_rfcomm_channel(packet);
    }
    if (event_type == SDP_EVENT_QUERY_COMPLETE) {
        uint8_t status = sdp_event_query_complete_get_status(packet);
        if (status) {
            printf("WL: SDP query failed, status 0x%02x\n", sdp_event_query_complete_get_status(packet));
            return;
        }
        if (rfcomm_server_channel) {
            printf("WL: SDP query done, channel %i\n", rfcomm_server_channel);
            rfcomm_create_channel(packet_handler, peer_addr, rfcomm_server_channel, NULL);
        } else {
            printf("WL: No SPP service found\n");
        }
    }
}

static void sdp_query(void *context) {
    if (state != QUERYING) return;
    state = CONNECTING;
    sdp_client_query_rfcomm_channel_and_name_for_uuid(
        &sdp_query_hander,
        peer_addr,
        BLUETOOTH_ATTRIBUTE_PUBLIC_BROWSE_ROOT
    );
}

void wireless_host_init() {
    info("WL: Host init\n");
    cyw43_arch_init();
    cyw43_pm_value(CYW43_NO_POWERSAVE_MODE, 2000, 1, 1, 1);
    l2cap_init();
    rfcomm_init();
    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);
    gap_ssp_set_io_capability(SSP_IO_CAPABILITY_DISPLAY_YES_NO); // ???
	hci_power_control(HCI_POWER_ON);
    info("WL: Host loop\n");
    loop_setup();
    btstack_run_loop_execute();
}
