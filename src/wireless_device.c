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
    // if (!added) printf("WL: Cannot add into queue\n");
    if (!added) printf("Q");
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

static void queue_process() {
    // static uint32_t last = 0;
    // uint32_t now = time_us_32() / 1000;
    // uint32_t elapsed = now - last;
    // last = now;
    uint8_t kb_reports = 0;
    uint8_t m_reports = 0;
    hid_keyboard_report_temp kb_report;
    hid_mouse_custom_report_temp m_report;
    while(!queue_is_empty(get_core_queue())) {
        uint8_t entry[32];
        queue_remove_blocking(get_core_queue(), entry);
        uint8_t report_type = entry[0];
        if (report_type == REPORT_KEYBOARD) {
            kb_reports += 1;
            kb_report = *(hid_keyboard_report_temp*)&entry[1];
        }
        if (report_type == REPORT_MOUSE) {
            m_reports += 1;
            hid_mouse_custom_report_temp report = *(hid_mouse_custom_report_temp*)&entry[1];
            if (m_reports == 1) {
                m_report = report;
            } else {
                report.x += m_report.x;
                report.y += m_report.y;
                m_report = report;
            }
        }
    }
    if (kb_reports + m_reports == 0) return;
    uint8_t index = 0;
    uint8_t wl_report[40] = {0,};
    if (kb_reports > 0) {
        wl_report[index] = REPORT_KEYBOARD;
        index += 1;
        memcpy(&wl_report[index], (uint8_t*)&kb_report, sizeof(hid_keyboard_report_temp));
        index += sizeof(hid_keyboard_report_temp);
    }
    if (m_reports > 0) {
        wl_report[index] = REPORT_MOUSE;
        wl_report[index+1] = m_reports;
        index += 2;
        memcpy(&wl_report[index], (uint8_t*)&m_report, sizeof(hid_mouse_custom_report_temp));
        index += sizeof(hid_mouse_custom_report_temp);
        if (m_reports > 1) printf("%i ", m_reports);
    }
    rfcomm_send(cid, wl_report, index);
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
            printf("WL: Connected\n");
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
    loop();
    btstack_run_loop_execute();
}
