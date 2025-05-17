// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#pragma once

#define UART_RX_BUFFER_SIZE 1024
#define UART_CONTROL_BYTES  30, 29, 28

typedef enum _UART_AT {
    AT_HID = 1,
    AT_WEBUSB,
    AT_BATTERY,
} UART_AT;

void uart_listen_serial();
void uart_listen_serial_limited();

void uart_rx_buffer_init();
bool uart_rx_buffer_is_empty();
bool uart_rx_buffer_is_full();
uint16_t uart_rx_buffer_available();
void uart_rx_buffer_consume(uint16_t len);
bool uart_rx_buffer_putc(uint8_t byte);
uint8_t uart_rx_buffer_getc();
uint8_t uart_rx_buffer_peekc();
void uart_rx_buffer_get(uint8_t *dest, uint16_t len);
bool uart_rx_buffer_match(uint8_t *pattern, uint8_t len);
void uart_rx_irq_callback();


