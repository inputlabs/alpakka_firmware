// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <pico/time.h>
#include <pico/cyw43_arch.h>
#include <pico/rand.h>
#include <lwip/tcp.h>
#include <lwip/pbuf.h>
#include "wlan.h"
#include "hid.h"
#include "logging.h"
#include "loop.h"
#include "led.h"
#include "profile.h"

static bool device_connected = false;
static struct tcp_pcb *tcp;
static ip_addr_t host_addr;
static ip_addr_t device_addr;

static void tcp_error(void *arg, err_t error) {
    warn("WLAN: tcp_client_error %i\n", error);
}

static err_t tcp_client_connected(void *arg, struct tcp_pcb *tcparg, err_t error) {
    if (error) {
        info("WLAN: TCP Cannot connect\n");
        return error;
    } else {
        device_connected = true;
        profile_update_leds();
        info("WLAN: TCP connected\n");
        return ERR_OK;
    }
}

static err_t tcp_client_sent(void *arg, struct tcp_pcb *tcparg, u16_t len) {
    // printf("tcp_client_sent %u\n", len);
    return ERR_OK;
}

void wlan_client_init() {
    info("WLAN: Device init\n");
    cyw43_arch_init();
    cyw43_pm_value(CYW43_NO_POWERSAVE_MODE, 2000, 1, 1, 1);
    cyw43_arch_enable_sta_mode();

    led_static_mask(LED_NONE);
    led_blink_mask(LED_TRIANGLE_UP);
    led_set_mode(LED_MODE_BLINK);

    ipaddr_aton(HOST_ADDR, &host_addr);
    ipaddr_aton(DEVICE_ADDR, &device_addr);
    netif_set_ipaddr(netif_default, &device_addr);

    info("WLAN: Connecting to access point...\n");
    int8_t status = cyw43_arch_wifi_connect_timeout_ms(
        AP_NAME,
        AP_PW,
        CYW43_AUTH_WPA2_AES_PSK,
        AP_TIMEOUT
    );
    if (status == 0) {
        info("WLAN: Connected to access point\n");
    } else {
        error("WLAN: Cannot connect to access point (status: %i)\n", status);
        return;
    }

    // udp = udp_new();
    // bufd = pbuf_alloc(PBUF_TRANSPORT, UDP_PACKET_LEN+1, PBUF_RAM);

    info("Connecting to TCP server %s:%u\n", HOST_ADDR, HOST_PORT);
    tcp = tcp_new_ip_type(IPADDR_TYPE_ANY);
    tcp_nagle_disable(tcp);
    tcp_err(tcp, tcp_error);
    tcp_sent(tcp, tcp_client_sent);
    tcp_connect(tcp, &host_addr, HOST_PORT, tcp_client_connected);
}

void wlan_client_task() {
    cyw43_arch_poll();
}

void wlan_send(uint8_t report_id, void *report, uint8_t len) {
    // struct pbuf *buf = pbuf_alloc(PBUF_TRANSPORT, UDP_PACKET_LEN+1, PBUF_RAM);
    // uint8_t *payload = (uint8_t *)bufd->payload;
    // memset(payload, 0, PACKET_LEN+1);
    // memcpy(payload, packet, len);

    // int8_t error = udp_sendto(udp, bufd, &host_addr, UDP_HOST_PORT);


    uint8_t packet[32] = {0,};
    // packet[0] = REPORT_TIMESTAMP;
    // uint64_t now = get_system_clock();
    // for (uint8_t i=0; i<8; i++) {
    //     packet[1+i] = (now >> (8 * i)) & 255;
    // }
    // print_array(&packet[1], 8);

    packet[0] = report_id;
    memcpy(&packet[1], report, len);

    err_t error = tcp_write(tcp, packet, len+1, TCP_WRITE_FLAG_COPY);
    if (error) warn("WLAN: tcp_write error %i\n", error);
    error = tcp_output(tcp);
    if (error) warn("WLAN: tcp_output error %i\n", error);
}
