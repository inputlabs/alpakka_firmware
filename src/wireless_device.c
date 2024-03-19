// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <pico/time.h>
#include <pico/multicore.h>
#include <pico/flash.h>
#include <pico/util/queue.h>
#include <pico/rand.h>
#include <btstack.h>
#include "tusb_config.h"
#include "wireless.h"
#include "hid.h"
#include "led.h"
#include "profile.h"
#include "logging.h"

static uint16_t cid;
static uint8_t spp_service_buffer[150];
static btstack_packet_callback_registration_t hci_event_callback_registration;
static btstack_timer_source_t timer;

void wireless_queue_append(uint8_t report_type, void *report, uint8_t len) {
    uint8_t entry[32] = {report_type};
    memcpy(&entry[1], report, len);
    bool added = queue_try_add(get_core_queue(), entry);
    if (!added) printf("WL: Cannot add into queue\n");
}

static void request_can_send(btstack_timer_source_t *timer) {
    rfcomm_request_can_send_now_event(cid);
}

static void queue_process() {
    static uint32_t last = 0;
    uint32_t now = time_us_32() / 1000;
    uint32_t elapsed = now - last;
    last = now;

    uint8_t reporting_type = 0;
    uint8_t num_reports = 0;
    uint8_t mouse_buttons = 0;
    int16_t mouse_x = 0;
    int16_t mouse_y = 0;
    uint8_t kb_modifiers = 0;
    uint8_t kb_keys[6] = {0,};
    while(!queue_is_empty(get_core_queue())) {
        uint8_t entry[32];
        queue_peek_blocking(get_core_queue(), entry);
        uint8_t report_type = entry[0];
        if (!reporting_type) reporting_type = report_type;
        if (reporting_type != report_type) break;
        queue_remove_blocking(get_core_queue(), entry);
        num_reports += 1;
        if (report_type == REPORT_KEYBOARD) {
            reporting_type = REPORT_KEYBOARD;
            hid_keyboard_report_temp report = *(hid_keyboard_report_temp*)&entry[1];
            kb_modifiers = report.modifier;
            memcpy(kb_keys, report.keycode, 6);
        }
        if (report_type == REPORT_MOUSE) {
            reporting_type = REPORT_MOUSE;
            hid_mouse_custom_report_temp report = *(hid_mouse_custom_report_temp*)&entry[1];
            mouse_buttons = report.buttons;
            mouse_x += report.x;
            mouse_y += report.y;
        }
    }
    if (reporting_type == 0) {
        timer.process = &request_can_send;
        btstack_run_loop_set_timer(&timer, OFFSTREAM_INTERVAL);
        btstack_run_loop_add_timer(&timer);
    }
    if (reporting_type == REPORT_KEYBOARD) {
        uint8_t report[10] = {0xa1, REPORT_KEYBOARD, kb_modifiers, 0};
        memcpy(&report[4], kb_keys, 6);
        rfcomm_send(cid, report, sizeof(report));
        rfcomm_request_can_send_now_event(cid);
    }
    if (reporting_type == REPORT_MOUSE) {
        if (num_reports > 1) printf("%i-%lu ", elapsed, num_reports);
        uint8_t report[] = {0xa1, REPORT_MOUSE, mouse_buttons, mouse_x>>8, mouse_x, mouse_y>>8, mouse_y};
        rfcomm_send(cid, report, sizeof(report));
        rfcomm_request_can_send_now_event(cid);
    }
}

static void event_handler(uint8_t *packet, uint16_t size) {
    bd_addr_t event_addr;
    uint8_t rfcomm_channel;
    uint8_t event_type = hci_event_packet_get_type(packet);
    if (event_type == HCI_EVENT_PIN_CODE_REQUEST) {
        hci_event_pin_code_request_get_bd_addr(packet, event_addr);
        gap_pin_code_response(event_addr, "0000");
    }
    if (event_type == RFCOMM_EVENT_INCOMING_CONNECTION) {
        rfcomm_event_incoming_connection_get_bd_addr(packet, event_addr);
        rfcomm_channel = rfcomm_event_incoming_connection_get_server_channel(packet);
        cid = rfcomm_event_incoming_connection_get_rfcomm_cid(packet);
        printf("WL: Channel %i requested for %s\n", rfcomm_channel, bd_addr_to_str(event_addr));
        rfcomm_accept_connection(cid);
    }
    if (event_type == RFCOMM_EVENT_CHANNEL_OPENED) {
        uint8_t error = rfcomm_event_channel_opened_get_status(packet);
        if (!error) {
            printf("WL: Channel open succeeded\n");
            cid = rfcomm_event_channel_opened_get_rfcomm_cid(packet);
            // rfcomm_mtu = rfcomm_event_channel_opened_get_max_frame_size(packet);
            gap_discoverable_control(0);
            gap_connectable_control(0);
            rfcomm_request_can_send_now_event(cid);
            profile_update_leds();
        } else {
            printf("WL: Channel open failed (0x%02x)\n", error);
        }
    }
    if (event_type == RFCOMM_EVENT_CAN_SEND_NOW) {
        queue_process();
    }
    if (event_type == RFCOMM_EVENT_CHANNEL_CLOSED) {
        printf("WL: Channel closed\n");
        cid = 0;
        gap_discoverable_control(1);
        gap_connectable_control(1);
    }
}

static void packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
    if (packet_type == HCI_EVENT_PACKET) {
        event_handler(packet, size);
    }
    if (packet_type == RFCOMM_DATA_PACKET) {
        // Bidirectional.
    }
}

void wireless_client_init() {
    info("WL: Device init (core %i)\n", get_core_num());
    multicore_lockout_victim_init();
    flash_safe_execute_core_init();

    cyw43_arch_init();
    cyw43_pm_value(CYW43_NO_POWERSAVE_MODE, 2000, 1, 1, 1);

    l2cap_init();
    rfcomm_init();
    rfcomm_register_service(packet_handler, RFCOMM_CHANNEL, 0xffff);  // ???

    // SDP.
    sdp_init();
    memset(spp_service_buffer, 0, sizeof(spp_service_buffer));
    spp_create_sdp_record(
        spp_service_buffer,
        sdp_create_service_record_handle(),
        RFCOMM_CHANNEL,
        DEVICE_NAME
    );
    btstack_assert(de_get_len(spp_service_buffer) <= sizeof(spp_service_buffer));
    sdp_register_service(spp_service_buffer);

    // HCI.
    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);

    // GAP.
    gap_set_class_of_device(CLASS_OF_DEVICE);
    gap_ssp_set_io_capability(SSP_IO_CAPABILITY_DISPLAY_YES_NO);  // ???
    gap_set_local_name(DEVICE_NAME_AND_MAC);
    gap_discoverable_control(1);

	hci_power_control(HCI_POWER_ON);

    led_static_mask(LED_NONE);
    led_blink_mask(LED_ALL);
    led_set_mode(LED_MODE_BLINK);

    info("WL: Device loop\n");
    btstack_run_loop_execute();
}
