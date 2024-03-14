// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include <pico/stdlib.h>
#include <pico/binary_info.h>
#include <pico/time.h>
#include <hardware/spi.h>
#include <btstack.h>
#include "wireless.h"
#include "hid.h"
#include "logging.h"
#include "ctrl.h" // system clock

// TODO move to headers
#define BUF_LEN         0x1
#define MAX_ATTRIBUTE_VALUE_SIZE 300

static btstack_packet_callback_registration_t hci_event_callback_registration;
static uint8_t hid_descriptor_storage[MAX_ATTRIBUTE_VALUE_SIZE];
static hid_protocol_mode_t hid_host_report_mode = HID_PROTOCOL_MODE_REPORT_WITH_FALLBACK_TO_BOOT;
static uint16_t hid_host_cid = 0;
static bool hid_host_descriptor_available = false;
static bd_addr_t remote_addr;

static const char * remote_addr_string = "28:CD:C1:06:C5:D5";  // Alpakka
// static const char * remote_addr_string = "DC:2C:26:AC:EA:A1";  // Uhuru
// static const char * remote_addr_string = "28:CD:C1:06:C5:D4";  // other pico

uint16_t period = 4;  // ms
static btstack_timer_source_t loop_timer;

static void loop_task(btstack_timer_source_t *ts){
    // if (get_system_clock()) return;
    btstack_run_loop_set_timer(ts, period);
    btstack_run_loop_add_timer(ts);
    host_task();
}

static void loop_setup(void){
    info("BT: Device loop setup\n");
    loop_timer.process = &loop_task;
    btstack_run_loop_set_timer(&loop_timer, period);
    btstack_run_loop_add_timer(&loop_timer);
}

void subevent_report(uint8_t *packet, uint16_t size) {
    const uint8_t *report = hid_subevent_report_get_report(packet);
    // if (report[1] == 0x01) {
    //     for(uint8_t i=0; i<6; i++) {
    //         hid_press_state(report[4+i], true);
    //     }
    // }
    if (report[1] == 0x02) {
        uint8_t buttons = report[2];
        int16_t x = (report[3] << 8) + report[4];
        int16_t y = (report[5] << 8) + report[6];

        // uint32_t sent = 0;
        // for(uint8_t i=0; i<8; i++) {
        //     sent += (((uint64_t)report[5+i]) << (8 * i));
        // }
        // uint32_t received = get_system_clock() + (time_us_32() / 1000);
        uint32_t received = time_us_32();

        static uint32_t last = 0;
        static uint32_t last_print = 0;
        static uint8_t num = 0;
        static uint16_t max = 0;


        uint16_t elapsed = (received-last) / 1000;
        if (elapsed > max) max = elapsed;
        num += 1;
        last = time_us_32();

        if(elapsed > 15) printf("%i ", elapsed);

        if(time_us_32()-last_print > 1000000) {
            last_print = time_us_32();
            info("num=%i max=%i\n", num, max);
            // info("avg=%0.2f sent=%i [%i %i %i %i %i] \n", avg/num, num, p10, p15, p20, p25, p30);
            // avg = 0;
            num = 0;
            max = 0;
            // p10 = p15 = p20 = p25 = p30 = 0;
        }

        // for(uint8_t i=0; i<6; i++) {
        //     hid_press_state(MOUSE_1+i, buttons & (1 << i));
        // }
        hid_report_direct_mouse(buttons, x, y, 0);
    }
    // hid_report();
}

void packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
    uint8_t event;
    uint8_t subevent;
    uint8_t status;

    if (packet_type == HCI_EVENT_PACKET) {
        event = hci_event_packet_get_type(packet);
        if(event == BTSTACK_EVENT_STATE) {
            uint8_t state = btstack_event_state_get_state(packet);
            if (state == HCI_STATE_WORKING) {
                info("BT: Host trying to connect to %s\n", bd_addr_to_str(remote_addr));
                status = hid_host_connect(remote_addr, hid_host_report_mode, &hid_host_cid);
                if (status != ERROR_CODE_SUCCESS) {
                    info("BT: Host connect failed, status 0x%02x\n", status);
                }
            } else {
                debug("BT: Undefined state: 0x%02x\n", state);
            }
        }
        else if (event == HCI_EVENT_HID_META) {
            subevent = hci_event_hid_meta_get_subevent_code(packet);
            if (subevent == HID_SUBEVENT_CONNECTION_OPENED) {
                status = hid_subevent_connection_opened_get_status(packet);
                if (status == ERROR_CODE_SUCCESS) {
                    hid_host_descriptor_available = false;
                    hid_host_cid = hid_subevent_connection_opened_get_hid_cid(packet);
                    info("BT: Host connected\n");
                    // period = 4000;
                } else {
                    hid_host_cid = 0;
                    info("BT: Host connection failed, status 0x%02x\n", status);
                }
            }
            else if (subevent == HID_SUBEVENT_CONNECTION_CLOSED) {
                info("BT: Host disconnected\n");
            }
            else if (subevent == HID_SUBEVENT_DESCRIPTOR_AVAILABLE) {
                status = hid_subevent_descriptor_available_get_status(packet);
                if (status == ERROR_CODE_SUCCESS){
                    hid_host_descriptor_available = true;
                } else {
                    info("BT: HID Descriptor is not available, status 0x%02x\n", status);
                }
            }
            else if (subevent == HID_SUBEVENT_REPORT) {
                if (hid_host_descriptor_available) {
                    subevent_report(packet, size);
                } else {
                    info("BT: No host descriptor\n");
                }
            } else {
                debug("BT: Undefined subevent: 0x%02x\n", subevent);
            }
        } else {
            debug("BT: Undefined event: 0x%02x\n", event);
        }
    } else {
        debug("BT: Undefined packet type: 0x%02x\n", packet_type);
    }
}

void wireless_host_init() {
    cyw43_arch_init();
    cyw43_pm_value(CYW43_NO_POWERSAVE_MODE, 2000, 1, 1, 1);
    l2cap_init();
    // Initialize HID Host
    hid_host_init(hid_descriptor_storage, sizeof(hid_descriptor_storage));
    hid_host_register_packet_handler(packet_handler);
    // Allow sniff mode requests by HID device and support role switch
    gap_set_default_link_policy_settings(
        LM_LINK_POLICY_ENABLE_SNIFF_MODE |
        LM_LINK_POLICY_ENABLE_ROLE_SWITCH
    );
    // try to become master on incoming connections
    hci_set_master_slave_policy(HCI_ROLE_MASTER);
    // register for HCI events
    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);
    // Disable stdout buffering //mp needed?
    setvbuf(stdout, NULL, _IONBF, 0);

    sscanf_bd_addr(remote_addr_string, remote_addr);
    hci_power_control(HCI_POWER_ON);
    info("BT: Host init completed\n");
    loop_setup();
    btstack_run_loop_execute();
}
