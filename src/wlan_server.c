// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <pico/time.h>
#include <pico/cyw43_arch.h>
#include <pico/rand.h>
#include <lwip/tcp.h>
#include <lwip/pbuf.h>
#include "wlan.h"
#include "hid.h"
#include "loop.h"
#include "logging.h"

static bool device_connected = false;
static struct tcp_pcb *tcp;
static ip_addr_t host_addr;
static ip_addr_t device_addr;

static void wlan_led_task() {
    static bool x;
    static uint32_t last = 0;
    uint32_t now = time_us_32() / 1000;
    uint16_t interval;
    if (device_connected) interval = DONGLE_LED_INTERVAL_CONNECTED;
    else interval = DONGLE_LED_INTERVAL_DISCONNECTED;
    if ((now - last) > interval) {
        x = !x;
        last = now;
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, x);
    }
}

static err_t tcp_server_receive(void *arg, struct tcp_pcb *tcparg, struct pbuf *buf, err_t error) {
    static uint64_t last = 0;
    uint64_t now = get_system_clock();
    uint64_t elapsed = now - last;
    last = now;
    // printf("%llu ", elapsed);

    if (buf->tot_len > 0) {
        // print_array(buf->payload, 20);
        uint8_t *payload = (uint8_t*)buf->payload;
        tcp_recved(tcparg, buf->tot_len);
        uint8_t report_type = payload[0];
        uint8_t entry[REPORT_QUEUE_ITEM_SIZE] = {0,};
        // if (report_type == REPORT_TIMESTAMP) {
        //     uint64_t ts;
        //     memcpy(&ts, &payload[1], 8);
        //     printf("%llu ", now-ts);
        // }
        if (report_type == REPORT_KEYBOARD) {
            memcpy(entry, &payload[1], sizeof(KeyboardReport));
            queue_try_add(hid_get_kb_queue(), entry);
        }
        if (report_type == REPORT_MOUSE) {
            memcpy(entry, &payload[1], sizeof(MouseReport));
            queue_try_add(hid_get_mouse_queue(), entry);
        }
    }
    pbuf_free(buf);
    return ERR_OK;
}

static void tcp_error(void *arg, err_t error) {
    warn("WLAN: tcp_client_error %i\n", error);
}

static err_t tcp_server_accepted(void *arg, struct tcp_pcb *tcparg, err_t error) {
    if (error || !tcparg) {
        info("WLAN: TCP cannot connect\n");
        return error;
    }
    info("WLAN: TCP Connected\n");
    device_connected = true;
    tcp_recv(tcparg, tcp_server_receive);
    tcp_err(tcparg, tcp_error);
    return ERR_OK;
}

void wlan_server_init() {
    info("WLAN: Host init\n");
    cyw43_arch_init();
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    cyw43_arch_enable_ap_mode(AP_NAME, AP_PW, CYW43_AUTH_WPA2_AES_PSK);
    // netif_default->mtu = 64;

    // udp = udp_new();
    // udp_recv(udp, wlan_host_callback, NULL);
    // udp_bind(udp, IP_ANY_TYPE, UDP_HOST_PORT);
    // struct pbuf *buf = pbuf_alloc(PBUF_TRANSPORT, UDP_PACKET_LEN+1, PBUF_RAM);

    info("WLAN: Starting TCP server %s:%u\n", ip4addr_ntoa(netif_ip4_addr(netif_list)), HOST_PORT);
    tcp = tcp_new_ip_type(IPADDR_TYPE_ANY);
    tcp_nagle_disable(tcp);
    err_t error = tcp_bind(tcp, IP_ADDR_ANY, HOST_PORT);
    if (error) warn("WLAN: tcp_bind error %i\n", error);
    tcp = tcp_listen_with_backlog(tcp, 1);
    if (!tcp) warn("WLAN: cannot listen\n");
    tcp_accept(tcp, tcp_server_accepted);

}

void wlan_server_task() {
    cyw43_arch_poll();
    wlan_led_task();
}
