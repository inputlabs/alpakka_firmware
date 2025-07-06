// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#pragma once

#define UART_RX_BUFFER_SIZE 1024

// Sequence of control bytes (chosen because these are non-printable, rarely used ASCII codes).
#define UART_CONTROL_0 30
#define UART_CONTROL_1 29
#define UART_CONTROL_2 28
#define UART_CONTROL_BYTES  UART_CONTROL_0, UART_CONTROL_1, UART_CONTROL_2

#define AT_HEADER_LEN 4
#define AT_HID_LEN 32
#define AT_WEBUSB_LEN 64
#define AT_BATTERY_LEN 4
#define AT_USB_PROTOCOL_LEN 1
#define AT_PAYLOAD_MAX_LEN  (AT_HEADER_LEN + AT_WEBUSB_LEN)

typedef enum _UART_AT {
    AT_HID = 1,  // Keyboard, mouse or gamepad report (also Xinput).
    AT_WEBUSB,  // WebUSB relay.
    AT_BATTERY,  // Battery level.
    AT_USB_PROTOCOL,  // USB protocol (Windows/Linux/Genetic) automatic dongle sync.
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


