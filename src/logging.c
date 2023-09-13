// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include "logging.h"
#include "webusb.h"
#include "helper.h"

LogLevel logging_level = LOG_INFO;
bool logging_onloop = false;

void logging_set_level(LogLevel level) {
    logging_level = level;
}

bool logging_get_level() {
    return logging_level;
}

void logging_set_onloop(bool value) {
    logging_onloop = value;
}

bool logging_get_onloop() {
    return logging_onloop;
}

void logging_init() {
    if (logging_level < LOG_DEBUG) return;
    for(uint8_t i=0; i<80; i++) printf("_");
    printf("\n");
}

void write(char *msg, va_list args) {
    char formatted[256] = {0,};
    vsnprintf(formatted, 256, msg, args);
    printf(formatted);  // UART.
    webusb_write(formatted);  // WebUSB.
}

void info(char *msg, ...) {
    va_list va;
    va_start(va, 0);
    write(msg, va);
}

void debug(char *msg, ...) {
    if (logging_level < LOG_DEBUG) return;
    va_list va;
    va_start(va, 0);
    write(msg, va);
}

void loguart(char *msg, ...) {
    if (logging_level < LOG_DEBUG) return;
    va_list va;
    va_start(va, 0);
    vprintf(msg, va);  // UART.
}
