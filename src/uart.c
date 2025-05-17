// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include <stdbool.h>
#include <pico/stdio.h>
#include <pico/bootrom.h>
#include <hardware/watchdog.h>
#include "uart.h"
#include "config.h"
#include "self_test.h"
#include "logging.h"
#include "power.h"
#include "esp.h"

void uart_listen_serial_do(bool limited) {
    char input = getchar_timeout_us(0);
    if (input == 'R') {
        info("UART: Restart\n");
        power_restart();
    }
    if (input == 'B') {
        info("UART: Bootsel mode\n");
        power_bootsel();
    }

    if (limited) {
        return;
    }

    if (input == 'C') {
        info("UART: Calibrate\n");
        config_calibrate();
    }
    if (input == 'F') {
        info("UART: Reset to factory settings\n");
        config_reset_factory();
    }
    if (input == 'D') {
        info("UART: Reset config\n");
        config_reset_config();
    }
    if (input == 'P') {
        info("UART: Reset profiles\n");
        config_reset_profiles();
    }
    if (input == 'T') {
        info("UART: Self-test\n");
        self_test();
    }
}

void uart_listen_serial() {
    static uint16_t i = 0;
    i += 1;
    // Execute only once per second.
    if (i % CFG_TICK_FREQUENCY) return;
    uart_listen_serial_do(false);
}

void uart_listen_serial_limited() {
    uart_listen_serial_do(true);
}

// Ring buffer since RP2040 hardware FIFO is only 32 bytes.
uint8_t rx_buffer[UART_RX_BUFFER_SIZE];
static volatile uint16_t write_pos;
static volatile uint16_t read_pos;

void uart_rx_buffer_init() {
    write_pos = 0;
    read_pos = 0;
}

bool uart_rx_buffer_is_empty() {
    // printf("W %i R %i\n", write_pos, read_pos);
    return write_pos == read_pos;
}

bool uart_rx_buffer_is_full() {
    return ((write_pos + 1) % UART_RX_BUFFER_SIZE) == read_pos;
}

uint16_t uart_rx_buffer_available() {
    if (write_pos >= read_pos) {
        return write_pos - read_pos;
    } else {
        return UART_RX_BUFFER_SIZE - (read_pos - write_pos);
    }
}

void uart_rx_buffer_consume(uint16_t len) {
    read_pos = (read_pos + len) % UART_RX_BUFFER_SIZE;
}

bool uart_rx_buffer_putc(uint8_t byte) {
    if (uart_rx_buffer_is_full()) {
        warn("UART: RX buffer full\n");
        return false;
    }
    rx_buffer[write_pos] = byte;
    write_pos = (write_pos + 1) % UART_RX_BUFFER_SIZE;
    return true;
}

uint8_t uart_rx_buffer_getc() {
    uint8_t byte = rx_buffer[read_pos];
    uart_rx_buffer_consume(1);
    return byte;
}

uint8_t uart_rx_buffer_peekc() {
    return rx_buffer[read_pos];
}

void uart_rx_buffer_get(uint8_t *dest, uint16_t len) {
    for(uint8_t i=0; i<len; i++) {
        dest[i] = rx_buffer[read_pos + i];
    }
    uart_rx_buffer_consume(len);
}

bool uart_rx_buffer_match(uint8_t *pattern, uint8_t len) {
    if (len > uart_rx_buffer_available()) return false;
    for(uint8_t i=0; i<len; i++) {
        if (rx_buffer[read_pos+i] != pattern[i]) return false;
    }
    return true;
}

void uart_rx_irq_callback() {
    while(uart_is_readable(ESP_UART)) {
        char c = uart_getc(ESP_UART);
        uart_rx_buffer_putc(c);
    }
}
