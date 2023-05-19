// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include <pico/stdio.h>
#include <pico/bootrom.h>
#include <hardware/watchdog.h>
#include "config.h"
#include "self_test.h"

void uart_listen_char_do(bool limited) {
    char input = getchar_timeout_us(0);
    if (input == 'R') {
        printf("UART: Restart\n");
        watchdog_enable(100, false);
    }
    if (input == 'B') {
        printf("UART: Bootsel mode\n");
        reset_usb_boot(0, 0);
    }

    if (limited) {
        return;
    }

    if (input == 'C') {
        printf("UART: Calibrate\n");
        config_calibrate();
    }
    if (input == 'F') {
        printf("UART: Format NVM\n");
        config_write_init();
        config_print();
    }
    if (input == 'T') {
        printf("UART: Self-test\n");
        self_test();
    }
}

void uart_listen_char() {
    uart_listen_char_do(false);
}

void uart_listen_char_limited() {
    uart_listen_char_do(true);
}
