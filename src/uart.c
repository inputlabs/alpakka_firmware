// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include <pico/stdio.h>
#include <pico/bootrom.h>
#include <hardware/watchdog.h>
#include "config.h"
#include "self_test.h"
#include "logging.h"

void uart_listen_char_do(bool limited) {
    char input = getchar_timeout_us(0);
    if (input == 'R') {
        info("UART: Restart\n");
        config_reboot();
    }
    if (input == 'B') {
        info("UART: Bootsel mode\n");
        config_bootsel();
    }

    if (limited) {
        return;
    }

    if (input == 'C') {
        info("UART: Calibrate\n");
        config_calibrate();
    }
    if (input == 'F') {
        info("UART: Format NVM\n");
        config_factory();
    }
    if (input == 'T') {
        info("UART: Self-test\n");
        self_test();
    }
}

void uart_listen_char(uint16_t loop_index) {
    if (loop_index % CFG_TICK_FREQUENCY) return;  // Execute only once per second.
    uart_listen_char_do(false);
}

void uart_listen_char_limited() {
    uart_listen_char_do(true);
}
