// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#pragma once

#define AP_NAME "ILDONGLE"
#define AP_PW "12345678"
#define AP_TIMEOUT 10000
#define PACKET_LEN 48

#define HOST_ADDR "192.168.4.1"
#define HOST_PORT 8010

#define DEVICE_ADDR "192.168.4.10"

#define DONGLE_LED_INTERVAL_CONNECTED 500
#define DONGLE_LED_INTERVAL_DISCONNECTED 100

void wlan_client_init();
void wlan_client_task();

void wlan_server_init();
void wlan_server_task();

void wlan_send(uint8_t report_id, void *packet, uint8_t len);
