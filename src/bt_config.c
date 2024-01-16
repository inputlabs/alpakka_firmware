// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <bt_config.h>
#include "btstack.h"
#include "pico/cyw43_arch.h"
#include "logging.h"
#include "tusb_config.h"


static uint8_t hid_service_buffer[300];
static uint8_t device_id_sdp_service_buffer[100];

static uint8_t hid_boot_device = 0;
static uint16_t host_max_latency = 1600;
static uint16_t host_min_timeout = 3200;
static uint8_t send_buffer_storage[16];
static btstack_ring_buffer_t  send_buffer;

static uint16_t hid_cid;

static const char hid_device_name[] = "Input Labs Alpakka";

static btstack_packet_callback_registration_t hci_event_callback_registration;

// close to USB HID Specification 1.1, Appendix B.1
const uint8_t hid_descriptor_keyboard[] = {
  0x05, 0x01,                    // Usage Page (Generic Desktop)
  0x09, 0x06,                    // Usage (Keyboard)
  0xa1, 0x01,                    // Collection (Application)

  // Report ID
  0x85, REPORT_ID,               // Report ID

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
};

static void packet_handler(uint8_t packet_type, uint16_t channel, uint8_t * packet, uint16_t packet_size){
    uint8_t   event;
    uint8_t   subevent;
    uint8_t   status;

    debug("Packet type 0x%02x, ", packet_type);

    switch (packet_type) {                                                   // src/bluetooth.h
        case HCI_EVENT_PACKET:                                                 // 0x04
            debug("HCI_EVENT_PACKET\n");
            event = hci_event_packet_get_type(packet);
            debug("Event: 0x%02x, ", event);

            switch (event) {                                                    //src/btstack_defines.h
                case HCI_EVENT_CONNECTION_COMPLETE:                               // 0x03
                    debug("HCI_EVENT_CONNECTION_COMPLETE, doing nothing\n");
                    break;
                case HCI_EVENT_CONNECTION_REQUEST:                                // 0x04
                    debug("HCI_EVENT_CONNECTION_REQUEST, doing nothing\n");
                    break;
                case HCI_EVENT_DISCONNECTION_COMPLETE:                                // 0x05
                    debug("HCI_EVENT_DISCONNECTION_COMPLETE, doing nothing\n");
                    break;
                case HCI_EVENT_ENCRYPTION_CHANGE:                                 // 0x08
                    debug("HCI_EVENT_ENCRYPTION_CHANGE, doing nothing\n");
                    break;
                case HCI_EVENT_READ_REMOTE_SUPPORTED_FEATURES_COMPLETE:           // 0x0B
                    debug("HCI_EVENT_READ_REMOTE_SUPPORTED_FEATURES_COMPLETE, doing nothing\n");
                    break;
                case HCI_EVENT_COMMAND_COMPLETE:                                  // 0x0E
                    debug("HCI_EVENT_COMMAND_COMPLETE, doing nothing\n");
                    break;
                case HCI_EVENT_COMMAND_STATUS:                                    // 0x0F
                    debug("HCI_EVENT_COMMAND_STATUS, doing nothing\n");
                    break;
                case HCI_EVENT_NUMBER_OF_COMPLETED_PACKETS:                       // 0x13
                    debug("HCI_EVENT_NUMBER_OF_COMPLETED_PACKETS, doing nothing\n");
                    break;
                case HCI_EVENT_LINK_KEY_REQUEST:                                  // 0x17
                    debug("HCI_EVENT_LINK_KEY_REQUEST, doing nothing\n");
                    break;
                case HCI_EVENT_LINK_KEY_NOTIFICATION:                             // 0x18
                    debug("HCI_EVENT_LINK_KEY_NOTIFICATION, doing nothing\n");
                    break;
                case HCI_EVENT_MAX_SLOTS_CHANGED:                                 // 0x1B
                    debug("HCI_EVENT_MAX_SLOTS_CHANGED, doing nothing\n");
                    break;
                case HCI_EVENT_READ_REMOTE_EXTENDED_FEATURES_COMPLETE:            // 0x23
                    debug("HCI_EVENT_READ_REMOTE_EXTENDED_FEATURES_COMPLETE, doing nothing\n");
                    break;
                case HCI_EVENT_IO_CAPABILITY_REQUEST:                             // 0x31
                    debug("HCI_EVENT_IO_CAPABILITY_REQUEST, doing nothing\n");
                    break;
                case HCI_EVENT_IO_CAPABILITY_RESPONSE:                             // 0x32
                    debug("HCI_EVENT_IO_CAPABILITY_RESPONSE, doing nothing\n");
                    break;
                case HCI_EVENT_USER_CONFIRMATION_REQUEST:                             // 0x33
                    debug("HCI_EVENT_USER_CONFIRMATION_REQUEST, doing nothing\n");
                    break;
                case HCI_EVENT_SIMPLE_PAIRING_COMPLETE:                            // 0x36
                    debug("HCI_EVENT_SIMPLE_PAIRING_COMPLETE, doing nothing\n");
                    break;

                case BTSTACK_EVENT_STATE:                                         // 0x60
                    uint8_t state = btstack_event_state_get_state(packet);
                    debug("BTSTACK_EVENT_STATE\n");
                    debug("State: 0x%02x, ", state);

                    switch (state) {                                                // src/hci_cmd.h
                        case HCI_STATE_INITIALIZING:                                  // 0x1
	                        debug("HCI_STATE_INITIALIZING, doing nothing\n");
                            return;
	                        break;
                        case HCI_STATE_WORKING:                                       // 0x2
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
                case BTSTACK_EVENT_NR_CONNECTIONS_CHANGED:                        // 0x61
                    debug("BTSTACK_EVENT_NR_CONNECTIONS_CHANGED, doing nothing\n");
                    break;
                case BTSTACK_EVENT_SCAN_MODE_CHANGED:                             // 0x66
                    debug("BTSTACK_EVENT_SCAN_MODE_CHANGED, doing nothing\n");
                    break;
                case HCI_EVENT_TRANSPORT_PACKET_SENT:                             // 0x6E
                    debug("HCI_EVENT_TRANSPORT_PACKET_SENT, doing nothing\n");
                    break;

                case GAP_EVENT_SECURITY_LEVEL:                                    // 0xD8
                    debug("GAP_EVENT_SECURITY_LEVEL, doing nothing\n");
                    break;
                case GAP_EVENT_PAIRING_STARTED:                                    // 0xE0
                    debug("GAP_EVENT_PAIRING_STARTED, doing nothing\n");
                    break;
                case GAP_EVENT_PAIRING_COMPLETE:                                    // 0xE1
                    debug("GAP_EVENT_PAIRING_COMPLETE, doing nothing\n");
                    break;
                case HCI_EVENT_HID_META:                                          // 0xEF
                    debug("HCI_EVENT_HID_META\n");
                    subevent = hci_event_hid_meta_get_subevent_code(packet);
                    debug("Subevent: 0x%02x, ", subevent);

                    switch (subevent){
                        case HID_SUBEVENT_CONNECTION_OPENED:                          // 0x02
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
                            debug("HID Connected\n");
			                hid_device_request_can_send_now_event(hid_cid);

                            gap_discoverable_control(0); // disabling to reduce latency
                            gap_connectable_control(0);  // disabling to reduce latency
	                        break;
                        case HID_SUBEVENT_CONNECTION_CLOSED:                          // 0x03
	                        debug("HID_SUBEVENT_CONNECTION_CLOSED, doing nothing\n");
                            break;
                        case HID_SUBEVENT_CAN_SEND_NOW:                               // 0x04
	                        debug("HID_SUBEVENT_CAN_SEND_NOW\n");

//                            uint8_t report[] = { 0xa1, REPORT_ID, 0, 0, keycode, 0, 0, 0, 0, 0};

                            debug("Sending report:\n");
//                            printf_hexdump(report, sizeof(report));
//                            debug("Sending keycode %d via BT\n", keycode);

	                        break;
                        case HID_SUBEVENT_SNIFF_SUBRATING_PARAMS:     // 0x0E
    	                    debug("HID_SUBEVENT_SNIFF_SUBRATING_PARAMS, doing nothing\n");
	                        break;
                        default:
	                        debug("\n---\n!!! Defaulting on subevent 0x%02x\n---\n", subevent);
	                        break;
                    }

                    break;
                case HCI_EVENT_VENDOR_SPECIFIC:                                   // 0xFF
                    debug("HCI_EVENT_VENDOR_SPECIFIC, doing nothing\n");
                    break;
                default:
                    debug("\n---\n!!! Defaulting on event type 0x%02x\n---\n", event);
                    break;
            }

            break;
        default:
            debug("\n---\n!!! Defaulting on packet type 0x%02x\n---\n", packet_type);
            break;
    }
}

void bt_hid_device_setup(void){
    if (cyw43_arch_init()) {
        error("failed to initialise cyw43_arch. Not a pico-w?\n");
        return -1;
    }

    // allow to get found by inquiry
    gap_discoverable_control(1);
    // use Limited Discoverable Mode; Peripheral; Keyboard as CoD
    gap_set_class_of_device(0x2540);
    // set local name to be identified - zeroes will be replaced by actual BD ADDR
    gap_set_local_name("Input Labs Alpakka 00:00:00:00:00:00");
    // allow for role switch in general and sniff mode
    gap_set_default_link_policy_settings( LM_LINK_POLICY_ENABLE_ROLE_SWITCH | LM_LINK_POLICY_ENABLE_SNIFF_MODE );
    // allow for role switch on outgoing connections - this allow HID Host to become master when we re-connect to it
    gap_set_allow_role_switch(true);

    debug("BT: init l2cap\n");
    l2cap_init();

    debug("BT: init service discovery\n");
    sdp_init();

    memset(hid_service_buffer, 0, sizeof(hid_service_buffer));
  
    uint8_t hid_virtual_cable = 0;
    uint8_t hid_remote_wake = 1;
    uint8_t hid_reconnect_initiate = 1;
    uint8_t hid_normally_connectable = 1;

    hid_sdp_record_t hid_params = {
    // hid sevice subclass 2540 Keyboard, hid counntry code 33 US, hid virtual cable off, hid reconnect initiate off, hid boot device off
    // hid service subclass of Mouse may also be 0x2540.
        0x2540, 33, 
        hid_virtual_cable, hid_remote_wake, 
        hid_reconnect_initiate, hid_normally_connectable,
        hid_boot_device,
        host_max_latency, host_min_timeout,
        3200,
        hid_descriptor_keyboard,
        sizeof(hid_descriptor_keyboard),
        hid_device_name
    };

    device_id_create_sdp_record(device_id_sdp_service_buffer, sdp_create_service_record_handle(), USB_GENERIC_VENDOR, USB_GENERIC_PRODUCT, 1, 1);
    btstack_assert(de_get_len( device_id_sdp_service_buffer) <= sizeof(device_id_sdp_service_buffer));
    sdp_register_service(device_id_sdp_service_buffer);

    // HID Device
    hid_device_init(hid_boot_device, sizeof(hid_descriptor_keyboard), hid_descriptor_keyboard);
       
    // register for HCI events
    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);

    // register for HID events
    hid_device_register_packet_handler(&packet_handler);

    btstack_ring_buffer_init(&send_buffer, send_buffer_storage, sizeof(send_buffer_storage));

    debug("BT: ready\n");

    hci_power_control(HCI_POWER_ON);
    
    btstack_run_loop_execute() ;

    debug("BT: running\n");
}
