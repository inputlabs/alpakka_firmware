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

// TODO move to headers
#define BUF_LEN         0x1
#define MAX_ATTRIBUTE_VALUE_SIZE 300

static btstack_packet_callback_registration_t hci_event_callback_registration;
static uint8_t hid_descriptor_storage[MAX_ATTRIBUTE_VALUE_SIZE];
static hid_protocol_mode_t hid_host_report_mode = HID_PROTOCOL_MODE_REPORT_WITH_FALLBACK_TO_BOOT;
static uint16_t hid_host_cid = 0;
static bool hid_host_descriptor_available = false;
static bd_addr_t remote_addr;

static bool connected = false;

static const char * remote_addr_string = "28:CD:C1:06:C5:D5";  // Alpakka
// static const char * remote_addr_string = "DC:2C:26:AC:EA:A1";  // Uhuru
// static const char * remote_addr_string = "28:CD:C1:06:C5:D4";  // other pico

void wireless_host_connect() {
    info("BT: Host trying to connect to %s\n", bd_addr_to_str(remote_addr));
    uint8_t status = hid_host_connect(remote_addr, hid_host_report_mode, &hid_host_cid);
    if (status != ERROR_CODE_SUCCESS) {
        info("BT: Host connect failed, status 0x%02x\n", status);
        // watchdog_enable(1, false);
    }
}

void wireless_led_task() {
    static uint32_t last = 0;
    uint32_t now = time_us_32() / 1000;
    uint16_t interval;
    if (connected) interval = 500;
    else interval = 100;
    if ((now - last) > interval) {
        static bool x;
        x = !x;
        last = now;
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, x);
    }
}

uint16_t period = 4;  // ms
static btstack_timer_source_t loop_timer;

static void loop_task(btstack_timer_source_t *ts){
    // if (get_system_clock()) return;
    btstack_run_loop_set_timer(ts, period);
    btstack_run_loop_add_timer(ts);
    wireless_led_task();
    host_task();
}

static void loop_setup(void){
    info("BT: Device loop setup\n");
    loop_timer.process = &loop_task;
    btstack_run_loop_set_timer(&loop_timer, period);
    btstack_run_loop_add_timer(&loop_timer);
}

void subevent_report(uint8_t *packet, uint16_t size) {
    uint32_t received = time_us_32();
    static uint32_t last = 0;
    static uint32_t last_print = 0;
    static uint16_t num = 0;
    static uint16_t max = 0;
    uint16_t elapsed = (received-last) / 1000;
    if (elapsed > max) max = elapsed;
    num += 1;
    last = time_us_32();
    if(elapsed > 5) printf("%i ", elapsed);
    if(time_us_32()-last_print > 1000000) {
        last_print = time_us_32();
        info("num=%i max=%i\n", num, max);
        num = max = 0;
    }
    const uint8_t *report = hid_subevent_report_get_report(packet);
    uint8_t report_type = report[1];
    if (report_type == REPORT_KEYBOARD) {
        uint8_t modifiers = report[2];
        uint8_t keys[6];
        memcpy(keys, &report[4], 6);
        hid_report_direct_keyboard(modifiers, keys);
    }
    if (report_type == REPORT_MOUSE) {
        uint8_t buttons = report[2];
        int16_t x = (report[3] << 8) + report[4];
        int16_t y = (report[5] << 8) + report[6];
        // int16_t scroll = report[7];
        hid_report_direct_mouse(buttons, x, y, 0);
    }
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
                wireless_host_connect();
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
                    connected = true;
                } else {
                    hid_host_cid = 0;
                    info("BT: Host connection failed, status 0x%02x\n", status);
                    // watchdog_enable(1, false);
                }
            }
            else if (subevent == HID_SUBEVENT_CONNECTION_CLOSED) {
                info("BT: Host disconnected\n");
                connected = false;
                // watchdog_enable(1, false);
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
                    // watchdog_enable(1, false);
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
    gap_set_default_link_policy_settings(LM_LINK_POLICY_ENABLE_SNIFF_MODE);
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
