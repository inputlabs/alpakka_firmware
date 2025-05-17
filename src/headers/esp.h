// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#pragma once
#include <hardware/uart.h>

#define ESP_UART uart1
#define ESP_BOOTLOADER_ADDR 0x0
#define ESP_PARTITION_ADDR 0x8000
#define ESP_FW_ADDR 0x10000
#define ESP_BOOTLOADER_BAUD 74880
#define ESP_FLASHER_BAUD 115200
#define ESP_FLASHER_BAUD_MAX (115200 * 4)
#define ESP_DATA_BAUD (115200 * 8)
#define ESP_RESTART_SETTLE 100  // Milliseconds.

void esp_init();
void esp_enable(bool state);
void esp_bootpin(bool state);
void esp_restart();
void esp_bootsel();
void esp_flash();
