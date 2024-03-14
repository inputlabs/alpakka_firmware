// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <pico/time.h>
#include <pico/util/queue.h>
#include <pico/rand.h>
#include <btstack.h>
#include "wireless.h"
#include "hid.h"
#include "ctrl.h" // system clock
#include "tusb_config.h"
#include "logging.h"

static uint8_t hid_service_buffer[400];
static uint8_t device_id_sdp_service_buffer[100];
static uint8_t send_buffer_storage[16];
static btstack_packet_callback_registration_t hci_event_callback_registration;
static uint16_t hid_cid;
static const char hid_device_name[] = "Input Labs Miau2";
// static btstack_ring_buffer_t send_buffer;

const uint8_t hid_descriptor[] = {
    // KEYBOARD
    0x05, 0x01,                    // Usage Page (Generic Desktop)
    0x09, 0x06,                    // Usage (Keyboard)
    0xa1, 0x02,                    // Collection (Logical)
    0x85, 0x01,                    // Report ID
    // Modifier byte (input)
    0x75, 0x01,                    //   Report Size (1)
    0x95, 0x08,                    //   Report Count (8)
    0x05, 0x07,                    //   Usage Page (Key codes)
    0x19, 0xe0,                    //   Usage Minimum (Keyboard LeftControl)
    0x29, 0xe7,                    //   Usage Maximum (Keyboard Right GUI)
    0x15, 0x00,                    //   Logical Minimum (0)
    0x25, 0x01,                    //   Logical Maximum (1)
    0x81, 0x02,                    //   Input (Data, Variable, Absolute)
    // Reserved byte (input)
    0x75, 0x01,                    //   Report Size (1)
    0x95, 0x08,                    //   Report Count (8)
    0x81, 0x03,                    //   Input (Constant, Variable, Absolute)
    // LED report + padding (output)
    0x95, 0x05,                    //   Report Count (5)
    0x75, 0x01,                    //   Report Size (1)
    0x05, 0x08,                    //   Usage Page (LEDs)
    0x19, 0x01,                    //   Usage Minimum (Num Lock)
    0x29, 0x05,                    //   Usage Maximum (Kana)
    0x91, 0x02,                    //   Output (Data, Variable, Absolute)
    0x95, 0x01,                    //   Report Count (1)
    0x75, 0x03,                    //   Report Size (3)
    0x91, 0x03,                    //   Output (Constant, Variable, Absolute)
    // Keycodes (input)
    0x95, 0x06,                    //   Report Count (6)
    0x75, 0x08,                    //   Report Size (8)
    0x15, 0x00,                    //   Logical Minimum (0)
    0x25, 0xff,                    //   Logical Maximum (1)
    0x05, 0x07,                    //   Usage Page (Key codes)
    0x19, 0x00,                    //   Usage Minimum (Reserved (no event indicated))
    0x29, 0xff,                    //   Usage Maximum (Reserved)
    0x81, 0x00,                    //   Input (Data, Array)
    0xc0,                          // End collection

    // MOUSE
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x02,                    // USAGE (Mouse)
    0xa1, 0x02,                    // COLLECTION (Logical)
    0x85, 0x02,                    // Report ID
    0x09, 0x01,                    //   USAGE (Pointer)
    0xa1, 0x00,                    //   COLLECTION (Physical)
    0x05, 0x09,                    //     USAGE_PAGE (Button)
    0x19, 0x01,                    //     USAGE_MINIMUM (Button 1)
    0x29, 0x03,                    //     USAGE_MAXIMUM (Button 3)
    0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //     LOGICAL_MAXIMUM (1)
    0x95, 0x03,                    //     REPORT_COUNT (3)
    0x75, 0x01,                    //     REPORT_SIZE (1)
    0x81, 0x02,                    //     INPUT (Data,Var,Abs)
    0x95, 0x01,                    //     REPORT_COUNT (1)
    0x75, 0x05,                    //     REPORT_SIZE (5)
    0x81, 0x03,                    //     INPUT (Cnst,Var,Abs)
    0x05, 0x01,                    //     USAGE_PAGE (Generic Desktop)
    0x09, 0x30,                    //     USAGE (X)
    0x09, 0x31,                    //     USAGE (Y)
    0x16, 0xff, 0x7f,              //     LOGICAL_MINIMUM
    0x26, 0x01, 0x80,              //     LOGICAL_MAXIMUM
    0x75, 0x08,                    //     REPORT_SIZE (16)
    0x95, 0x02,                    //     REPORT_COUNT (2)
    0x81, 0x06,                    //     INPUT (Data,Var,Rel)
    0xc0,                          //   END_COLLECTION
    0xc0                           // END_COLLECTION
};

void wireless_process_queue() {
    uint8_t n = 0;
    uint8_t buttons = 0;
    int16_t x = 0;
    int16_t y = 0;
    while(!queue_is_empty(get_core_queue())) {
        n++;
        uint8_t report[32];
        queue_remove_blocking(get_core_queue(), report);
        if (report[0] == 2) {
            buttons = report[1];
            x += (report[2] << 8) + report[3];
            y += (report[4] << 8) + report[5];
        }
    }
    if (n>1) printf("%i ", n);
    // if (abs(x) > 127) printf()
    if (n) wireless_report_mouse(buttons, x, y);
}

void wireless_report_keyboard(int8_t modifiers, int8_t k[6]) {
    uint8_t report[] = {0xa1, 0x01, modifiers, 0, k[0], k[1], k[2], k[3], k[4], k[5]};
    hid_device_send_interrupt_message(hid_cid, &report[0], sizeof(report));
}

void wireless_report_mouse(int8_t buttons, int16_t x, int16_t y) {
    // uint64_t rt = get_system_clock() + (time_us_32() / 1000);

    // x = get_rand_32() % 21 - 10;
    // y = get_rand_32() % 21 - 10;

    // x = y = 0;
    // static uint8_t z = 0;
    // z += 8;
    // if (z < 127) x = 40;
    // else x = -40;

    uint8_t report[] = {
        0xa1,
        0x02,
        buttons,
        x >> 8,
        x,
        y >> 8,
        y
        // (rt >> (8*0)) & 255,
        // (rt >> (8*1)) & 255,
        // (rt >> (8*2)) & 255,
        // (rt >> (8*3)) & 255,
        // (rt >> (8*4)) & 255,
        // (rt >> (8*5)) & 255,
        // (rt >> (8*6)) & 255,
        // (rt >> (8*7)) & 255,
    };
    hid_device_send_interrupt_message(hid_cid, &report[0], sizeof(report));
}

#define PERIOD 1  // ms
static btstack_timer_source_t loop_timer;

static void loop_task(btstack_timer_source_t *ts){
    btstack_run_loop_set_timer(ts, PERIOD);
    btstack_run_loop_add_timer(ts);
    if (hid_cid) {
        hid_device_request_can_send_now_event(hid_cid);
    }
}

static void loop_setup(void){
    info("BT: Device loop setup\n");
    loop_timer.process = &loop_task;
    btstack_run_loop_set_timer(&loop_timer, PERIOD);
    btstack_run_loop_add_timer(&loop_timer);
}


static void packet_handler(uint8_t packet_type, uint16_t channel, uint8_t * packet, uint16_t packet_size){
    uint8_t   event;
    uint8_t   subevent;
    uint8_t   status;

    debug("Packet type 0x%02x, ", packet_type);

    switch (packet_type) {
        case HCI_EVENT_PACKET:                                                 // 0x04
            debug("HCI_EVENT_PACKET\n");
            event = hci_event_packet_get_type(packet);
            debug("Event: 0x%02x, ", event);

            switch (event) {

                case BTSTACK_EVENT_STATE:                                          // 0x60
                    uint8_t state = btstack_event_state_get_state(packet);
                    debug("BTSTACK_EVENT_STATE\n");
                    debug("State: 0x%02x, ", state);

                    switch (state) {                                               // src/hci_cmd.h
                        case HCI_STATE_INITIALIZING:                               // 0x1
	                        debug("HCI_STATE_INITIALIZING, doing nothing\n");
                            return;
	                        break;
                        case HCI_STATE_WORKING:                                    // 0x2
    	                    debug("HCI_STATE_WORKING\n");
	                        debug("!!! Waiting for connection\n");
                            app_state = APP_NOT_CONNECTED;
	                        break;
                        default:
	                        debug("\n---\n!!! Defaulting on state 0x%02x\n---\n", state);
                            return;
    	                    break;
                    }

                    break;

                case HCI_EVENT_HID_META:                                           // 0xEF
                    debug("HCI_EVENT_HID_META\n");
                    subevent = hci_event_hid_meta_get_subevent_code(packet);
                    debug("Subevent: 0x%02x, ", subevent);

                    switch (subevent){
                        case HID_SUBEVENT_CONNECTION_OPENED:                       // 0x02
	                        debug("HID_SUBEVENT_CONNECTION_OPENED\n");
                            status = hid_subevent_connection_opened_get_status(packet);

                            if (status != ERROR_CODE_SUCCESS) {
                                debug("Connection failed, status 0x%x\n", status);
                                app_state = APP_NOT_CONNECTED;
                                hid_cid = 0;
                                return;
                            }
                            app_state = APP_CONNECTED;
                            hid_cid = hid_subevent_connection_opened_get_hid_cid(packet);
                            info("BT: Client connected\n");
			                hid_device_request_can_send_now_event(hid_cid);
                            gap_discoverable_control(0); // disabling to reduce latency
                            gap_connectable_control(0);  // disabling to reduce latency
                            // loop_setup();
	                        break;
                        case HID_SUBEVENT_CONNECTION_CLOSED:                       // 0x03
	                        debug("HID_SUBEVENT_CONNECTION_CLOSED, doing nothing\n");
                            break;
                        case HID_SUBEVENT_CAN_SEND_NOW:                            // 0x04
	                        debug("HID_SUBEVENT_CAN_SEND_NOW\n");
                            // wireless_report_mouse(0, 0, 0);
                            wireless_process_queue();
	                        break;
                        case HID_SUBEVENT_SNIFF_SUBRATING_PARAMS:     // 0x0E
    	                    debug("HID_SUBEVENT_SNIFF_SUBRATING_PARAMS, doing nothing\n");
	                        break;
                        default:
	                        debug("\n---\n!!! Defaulting on subevent 0x%02x\n---\n", subevent);
	                        break;
                    }

                    break;
            }

            break;
        default:
            debug("\n---\n!!! Defaulting on packet type 0x%02x\n---\n", packet_type);
            break;
    }
}

void wireless_client_init() {
    cyw43_arch_init();
    cyw43_pm_value(CYW43_NO_POWERSAVE_MODE, 2000, 1, 1, 1);
    gap_discoverable_control(1);
    gap_set_class_of_device(0x2580);
    gap_set_local_name("Input Labs Miau 00:00:00:00:00:00");
    gap_set_default_link_policy_settings( LM_LINK_POLICY_ENABLE_ROLE_SWITCH | LM_LINK_POLICY_ENABLE_SNIFF_MODE );
    gap_set_allow_role_switch(true);
    l2cap_init();
    sdp_init();
    memset(hid_service_buffer, 0, sizeof(hid_service_buffer));

    uint8_t hid_boot_device = 0;

    hid_sdp_record_t hid_params = {
        0x2580, // Device class.
        40,  // Country code.
        0,  // hid_virtual_cable.
        1,  // hid_remote_wake.
        1,  // hid_reconnect_initiate.
        1,  // hid_normally_connectable.
        hid_boot_device,
        4000,  // host_max_latency.
        3000,  // host_min_timeout.
        3000,  // supervision_timeout.
        hid_descriptor,
        sizeof(hid_descriptor),
        hid_device_name
    };

    hid_create_sdp_record(hid_service_buffer, sdp_create_service_record_handle(), &hid_params);
    btstack_assert(de_get_len( hid_service_buffer) <= sizeof(hid_service_buffer));
    sdp_register_service(hid_service_buffer);


    device_id_create_sdp_record(device_id_sdp_service_buffer, sdp_create_service_record_handle(), USB_GENERIC_VENDOR, USB_GENERIC_PRODUCT, 1, 1);
    btstack_assert(de_get_len( device_id_sdp_service_buffer) <= sizeof(device_id_sdp_service_buffer));
    sdp_register_service(device_id_sdp_service_buffer);

    // HID Device
    hid_device_init(hid_boot_device, sizeof(hid_descriptor), hid_descriptor);

    // register for HCI events
    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);

    // register for HID events
    hid_device_register_packet_handler(&packet_handler);

    // btstack_ring_buffer_init(&send_buffer, send_buffer_storage, sizeof(send_buffer_storage));

    info("BT: Device init completed (core %i)\n", get_core_num());
    hci_power_control(HCI_POWER_ON);
    info("BT: Device powered on\n");
    loop_setup();
    btstack_run_loop_execute();
}
