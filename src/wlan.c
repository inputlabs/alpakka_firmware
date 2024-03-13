// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <pico/time.h>
#include <pico/cyw43_arch.h>
#include <pico/rand.h>
#include <lwip/udp.h>
#include <lwip/pbuf.h>
#include "hid.h"
#include "ctrl.h" // system clock
#include "logging.h"

#define AP_NAME "ILDONGLE"
#define AP_PW "12345678"
#define AP_TIMEOUT 10000
#define UDP_DEVICE_ADDR "192.168.1.10"
#define UDP_HOST_ADDR "255.255.255.255"
#define UDP_HOST_PORT 8010
#define UDP_PACKET_LEN 32

bool device_connected = false;
struct udp_pcb* udp;
struct pbuf *bufd;
ip_addr_t host_addr;
ip_addr_t device_addr;

void wlan_host_callback(void *arg, struct udp_pcb *pcb, struct pbuf *buf, const ip_addr_t *addr, u16_t port) {
    cyw43_arch_lwip_begin();
    uint8_t *report = (uint8_t *)buf->payload;
    // uint64_t received = get_system_clock() + (time_us_32() / 1000);
    // uint64_t sent = 0;
    // for(uint8_t i=0; i<8; i++) {
    //     sent += (((uint64_t)report[0+i]) << (8 * i));
    // }
    // uint64_t latency = received - sent;
    // info("%llu\n", latency);

    uint32_t current;
    current = time_us_32();
    static uint32_t last;
    static uint32_t last_printed;
    static uint32_t max;
    static uint16_t num = 0;
    uint32_t elapsed = current - last;
    if (elapsed > max) max = elapsed;
    num += 1;
    if (current-last_printed > 1000000) {
        info("num:%i, me:%i\n", num, max);
        max = 0;
        num = 0;
        last_printed = current;
    }
    last = current;

    uint8_t report_type = report[0];
    if (report_type == 1) {
        uint8_t modifiers = report[1];
        uint8_t keys[6] = {0};
        memcpy(keys, &report[3], 6);
        pbuf_free(buf);
        hid_report_direct_keyboard(modifiers, keys);
    }
    if (report_type == 2) {
        uint8_t buttons = report[1];
        int8_t x = report[2];
        int8_t y = report[3];
        // info("%i %i\n", x, y);
        pbuf_free(buf);
        hid_report_direct_mouse(buttons, x, y, 0);
    }
    cyw43_arch_lwip_end();
}

void wlan_host_setup() {
    info("WLAN: Wireless host setup\n");
    cyw43_arch_init();
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    cyw43_arch_enable_ap_mode(AP_NAME, AP_PW, CYW43_AUTH_WPA2_AES_PSK);
    // netif_default->mtu = 64;
    udp = udp_new();
    udp_recv(udp, wlan_host_callback, NULL);
    udp_bind(udp, IP_ANY_TYPE, UDP_HOST_PORT);
    struct pbuf *buf = pbuf_alloc(PBUF_TRANSPORT, UDP_PACKET_LEN+1, PBUF_RAM);
}

void wlan_device_setup() {
    info("WLAN: Wireless device setup\n");
    cyw43_arch_init();
    cyw43_pm_value(CYW43_NO_POWERSAVE_MODE, 2000, 1, 1, 1);
    cyw43_arch_enable_sta_mode();

    ipaddr_aton(UDP_HOST_ADDR, &host_addr);
    ipaddr_aton(UDP_DEVICE_ADDR, &device_addr);
    netif_set_ipaddr(netif_default, &device_addr);
    // netif_default->mtu = 64;

    info("WLAN: Connecting to access point...\n");
    int8_t status = cyw43_arch_wifi_connect_timeout_ms(
        AP_NAME,
        AP_PW,
        CYW43_AUTH_WPA2_AES_PSK,
        AP_TIMEOUT
    );
    if (status == 0) {
        device_connected = true;
        info("WLAN: Connected to access point\n");
    } else {
        device_connected = false;
        error("WLAN: Cannot connect to access point (status: %i)\n", status);
    }
    udp = udp_new();
    bufd = pbuf_alloc(PBUF_TRANSPORT, UDP_PACKET_LEN+1, PBUF_RAM);
}

void wlan_host_task() {
    // LED.
    static uint8_t i = 0;
    static bool x = false;
    i++;
    if (i == 20) {
        i = 0;
        x = !x;
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, x);
    }
    // Poll.
    cyw43_arch_poll();
}

void wlan_report_mouse(int8_t buttons, int8_t x, int8_t y); //////

void wlan_device_task() {
    if(!device_connected) {
        cyw43_arch_poll();
        return;
    }

    // static uint16_t i = 0;
    // i++;
    // uint8_t rx = get_rand_32() % 11 - 5;
    // uint8_t ry = get_rand_32() % 11 - 5;
    // wlan_report_mouse(0, rx, ry);

    hid_report_wireless_device();
    cyw43_arch_poll();
}

void wlan_report(uint8_t *report, uint8_t len) {
    // struct pbuf *buf = pbuf_alloc(PBUF_TRANSPORT, UDP_PACKET_LEN+1, PBUF_RAM);
    uint8_t *payload = (uint8_t *)bufd->payload;
    memset(payload, 0, UDP_PACKET_LEN+1);
    memcpy(payload, report, len);
    int8_t error = udp_sendto(udp, bufd, &host_addr, UDP_HOST_PORT);
    if (!error) {
        static uint8_t sent = 0;
        sent++;
        if (sent == 255) printf("Sent 255 %lu\n", time_us_32());
    }
    else {
        printf("WLAN: UDP send error %i\n", error);
        // printf("X");
    }
    // pbuf_free(buf);

    // uint64_t rt = get_system_clock() + (time_us_32() / 1000);
    // uint8_t report[] = {
    //     // 0xa1,
    //     // 0x02,
    //     // buttons,
    //     // x,
    //     // y,
    //     (rt >> (8*0)) & 255,
    //     (rt >> (8*1)) & 255,
    //     (rt >> (8*2)) & 255,
    //     (rt >> (8*3)) & 255,
    //     (rt >> (8*4)) & 255,
    //     (rt >> (8*5)) & 255,
    //     (rt >> (8*6)) & 255,
    //     (rt >> (8*7)) & 255,
    // };
}

// TODO merge with hid.
void wlan_report_keyboard(uint8_t modifiers, uint8_t k[6]) {
    uint8_t report[10] = {1, modifiers, 0, k[0], k[1], k[2], k[3], k[4], k[5]};
    wlan_report(report, 10);
}

void wlan_report_mouse(int8_t buttons, int8_t x, int8_t y) {
    uint8_t report[4] = {2, buttons, x, y};
    wlan_report(report, 4);
}
