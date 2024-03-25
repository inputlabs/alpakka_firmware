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
#include "xinput.h"
#include "hid.h"
#include "led.h"
#include "profile.h"
#include "logging.h"

static uint16_t cid;
static uint8_t spp_service_buffer[150];
static btstack_packet_callback_registration_t hci_event_callback_registration;
static btstack_timer_source_t timer;

bool wireless_device_is_connected() {
    return !!cid;
}

static void request_can_send(btstack_timer_source_t *timer) {
    rfcomm_request_can_send_now_event(cid);
}

void loop() {
    timer.process = &loop;
    btstack_run_loop_set_timer(&timer, OFFSTREAM_INTERVAL);
    btstack_run_loop_add_timer(&timer);
    if (cid) rfcomm_request_can_send_now_event(cid);
}

static void event_pin_code_request_cb(uint8_t *packet) {
    bd_addr_t event_addr;
    hci_event_pin_code_request_get_bd_addr(packet, event_addr);
    gap_pin_code_response(event_addr, "0000");
}

static void event_incomming_connection_cb(uint8_t *packet) {
    bd_addr_t event_addr;
    uint8_t rfcomm_channel;
    rfcomm_event_incoming_connection_get_bd_addr(packet, event_addr);
    rfcomm_channel = rfcomm_event_incoming_connection_get_server_channel(packet);
    cid = rfcomm_event_incoming_connection_get_rfcomm_cid(packet);
    printf("WL: Channel %i requested for %s\n", rfcomm_channel, bd_addr_to_str(event_addr));
    rfcomm_accept_connection(cid);
}

static void event_channel_opened_cb(uint8_t *packet) {
    uint8_t error = rfcomm_event_channel_opened_get_status(packet);
    if (error) {
        printf("WL: Channel open failed (0x%02x)\n", error);
        return;
    }
    printf("WL: Connected\n");
    cid = rfcomm_event_channel_opened_get_rfcomm_cid(packet);
    // rfcomm_mtu = rfcomm_event_channel_opened_get_max_frame_size(packet);
    gap_discoverable_control(0);
    gap_connectable_control(0);
    rfcomm_request_can_send_now_event(cid);
    profile_update_leds();
}

static void event_channel_closed_cb(uint8_t *packet) {
    printf("WL: Channel closed\n");
    cid = 0;
    gap_discoverable_control(1);
    gap_connectable_control(1);
}

// Queue to air.
static void event_can_send_now_cb(uint8_t *packet) {
    // static uint32_t last = 0;
    // uint32_t now = time_us_32() / 1000;
    // uint32_t elapsed = now - last;
    // last = now;
    uint8_t kb_reports = 0;  // Keyboard.
    uint8_t m_reports = 0;  // Mouse.
    uint8_t x_reports = 0;  // XInput.
    bool mouse_eot = false;  // Mouse End Of Transmission.
    KeyboardReport kb_report;
    MouseReport m_report;
    XInputReport x_report;
    // Merge reports by type.
    while(!queue_is_empty(get_core_queue())) {
        uint8_t entry[32];
        queue_remove_blocking(get_core_queue(), entry);
        uint8_t report_type = entry[0];
        if (report_type == REPORT_KEYBOARD) {
            kb_reports += 1;
            KeyboardReport report = *(KeyboardReport*)&entry[1];
            memcpy(&kb_report, &report, sizeof(KeyboardReport));
        }
        if (report_type == REPORT_MOUSE) {
            m_reports += 1;
            mouse_eot = false;
            MouseReport report = *(MouseReport*)&entry[1];
            if (m_reports > 1) {
                report.x += m_report.x;
                report.y += m_report.y;
            }
            memcpy(&m_report, &report, sizeof(MouseReport));
        }
        if (report_type == REPORT_XINPUT) {
            x_reports += 1;
            XInputReport report = *(XInputReport*)&entry[1];
            memcpy(&x_report, &report, sizeof(XInputReport));
        }
        if (report_type == REPORT_MOUSE_EOT) {
            mouse_eot = true;
        }
    }
    if (kb_reports + m_reports + x_reports == 0 && !mouse_eot) return;
    // Compose a combined report.
    uint8_t index = 0;
    uint8_t wl_report[48] = {0,};
    if (kb_reports > 0) {
        wl_report[index] = REPORT_KEYBOARD;
        index += 1;
        memcpy(&wl_report[index], (uint8_t*)&kb_report, sizeof(KeyboardReport));
        index += sizeof(KeyboardReport);
    }
    if (m_reports > 0) {
        wl_report[index] = REPORT_MOUSE;
        wl_report[index+1] = m_reports;
        index += 2;
        memcpy(&wl_report[index], (uint8_t*)&m_report, sizeof(MouseReport));
        index += sizeof(MouseReport);
        if (m_reports > 1) printf("%i ", m_reports);
    }
    if (x_reports > 0) {
        wl_report[index] = REPORT_XINPUT;
        index += 1;
        memcpy(&wl_report[index], (uint8_t*)&x_report, sizeof(XInputReport));
        index += sizeof(XInputReport);
    }
    if (mouse_eot) {
        wl_report[index] = REPORT_MOUSE_EOT;
        index += 1;
    }
    rfcomm_send(cid, wl_report, index);
}

static void event_handler(uint8_t *packet) {
    uint8_t event_type = hci_event_packet_get_type(packet);
    if (event_type == HCI_EVENT_PIN_CODE_REQUEST) event_pin_code_request_cb(packet);
    else if (event_type == RFCOMM_EVENT_INCOMING_CONNECTION) event_incomming_connection_cb(packet);
    else if (event_type == RFCOMM_EVENT_CHANNEL_OPENED) event_channel_opened_cb(packet);
    else if (event_type == RFCOMM_EVENT_CAN_SEND_NOW) event_can_send_now_cb(packet);
    else if (event_type == RFCOMM_EVENT_CHANNEL_CLOSED) event_channel_opened_cb(packet);
}

static void packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
    if (packet_type == HCI_EVENT_PACKET) event_handler(packet);
    // if (packet_type == RFCOMM_DATA_PACKET) data_packet_cb(packet, size);
}

void wireless_device_init() {
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
    loop();
    btstack_run_loop_execute();
}
