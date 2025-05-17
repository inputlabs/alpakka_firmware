// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include "esp.h"
#include "config.h"
#include "pin.h"
#include "power.h"

void esp_log_state() {
    #ifdef DEVICE_HAS_MARMOTA
        info(
            "ESP: enable=%i boot=%i\n",
            gpio_get(PIN_ESP_ENABLE),
            gpio_get(PIN_ESP_BOOT)
        );
    #endif
}

void esp_init() {
    #ifdef DEVICE_HAS_MARMOTA
        // Boot pin.
        gpio_init(PIN_ESP_BOOT);
        gpio_set_dir(PIN_ESP_BOOT, GPIO_OUT);
        gpio_put(PIN_ESP_BOOT, false);
        // Power enable pin.
        gpio_init(PIN_ESP_ENABLE);
        gpio_set_dir(PIN_ESP_ENABLE, GPIO_OUT);
        gpio_put(PIN_ESP_ENABLE, false);
        // Log.
        esp_log_state();
    #endif
}

void esp_enable(bool state) {
    #ifdef DEVICE_HAS_MARMOTA
        gpio_put(PIN_ESP_ENABLE, state);
        sleep_ms(ESP_RESTART_SETTLE);
    #endif
}

void esp_bootpin(bool state) {
    #ifdef DEVICE_HAS_MARMOTA
        gpio_put(PIN_ESP_BOOT, state);
        sleep_ms(ESP_RESTART_SETTLE);
    #endif
}

void esp_restart() {
    #ifdef DEVICE_HAS_MARMOTA
        info("ESP: Restart\n");
        esp_enable(false);
        esp_bootpin(true);
        esp_enable(true);
        esp_log_state();
    #endif
}

void esp_bootsel() {
    #ifdef DEVICE_HAS_MARMOTA
        info("ESP: Bootsel\n");
        esp_enable(false);
        esp_bootpin(false);
        esp_enable(true);
        esp_log_state();
    #endif
}

#ifdef DEVICE_LLAMA
    #include <esp-serial-flasher/include/esp_loader.h>
    #include <esp-serial-flasher/port/pi_pico_port.h>
    #include <esp-serial-flasher/examples/common/example_common.h>
    #include "../../esp_llama/build/llama.bin.c"
    // #include "../../esp_llama/build/llama_empty.bin.c"  // Empty version for faster development.

    void esp_flash() {
        info("RF: ESP flash start\n");
        uart_set_baudrate(ESP_UART, ESP_FLASHER_BAUD);
        const loader_pi_pico_config_t config = {
            .uart_inst = ESP_UART,
            .baudrate = ESP_FLASHER_BAUD,
            .uart_tx_pin_num = PIN_UART1_TX,
            .uart_rx_pin_num = PIN_UART1_RX,
            .reset_trigger_pin_num = PIN_ESP_ENABLE,
            .boot_pin_num = PIN_ESP_BOOT,
            .dont_initialize_peripheral = true,
        };
        loader_port_pi_pico_init(&config);
        // See https://docs.espressif.com/projects/esptool/en/latest/esp32/esptool/flasher-stub.html
        uint32_t connect = connect_to_target_with_stub(ESP_FLASHER_BAUD, ESP_FLASHER_BAUD_MAX);
        if (connect != ESP_LOADER_SUCCESS) {
            error("RF: ESP flasher cannot connect (error=%li)\n", connect);
            return;
        }
        flash_binary(ESP_BOOTLOADER, sizeof(ESP_BOOTLOADER), ESP_BOOTLOADER_ADDR);
        flash_binary(ESP_PARTITION, sizeof(ESP_PARTITION), ESP_PARTITION_ADDR);
        flash_binary(ESP_FIRMWARE, sizeof(ESP_FIRMWARE), ESP_FW_ADDR);
        // loader_port_pi_pico_deinit();
        printf("RF: ESP flash done\n");
        printf("RF: Going into bootsel mode...\n");
        power_bootsel();
    }
#endif
