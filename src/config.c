// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include <hardware/sync.h>
#include <hardware/watchdog.h>
#include <pico/bootrom.h>
#include "config.h"
#include "nvm.h"
#include "led.h"
#include "hid.h"
#include "imu.h"
#include "thumbstick.h"
#include "profile.h"
#include "helper.h"

uint8_t config_tune_mode = 0;

void config_read(config_nvm_t* config) {
    nvm_read(XIP_BASE + NVM_CONFIG_ADDR, (uint8_t*)config, 256);
}

void config_write(config_nvm_t* config) {
    nvm_write(NVM_CONFIG_ADDR, (uint8_t*)config, 256);
}

void config_write_init() {
    config_nvm_t config = {
        .header = NVM_CONFIG_HEADER,
        .config_version = CFG_STRUCT_VERSION,
        .os_mode = 0,
        .profile = 1,
        .sensitivity = 0,
        .deadzone = 0,
        .vibration = 0,
        .ts_offset_x = 0,
        .ts_offset_y = 0,
        .imu_0_offset_x = 0,
        .imu_0_offset_y = 0,
        .imu_1_offset_x = 0,
        .imu_1_offset_y = 0
    };
    config_write(&config);
}

void config_print() {
    config_nvm_t config;
    config_read(&config);
    printf("NVM:\n");
    printf("  config_version=%i\n", config.config_version);
    printf("  os_mode=%i\n", config.os_mode);
    printf("  profile=%i\n", config.profile);
    printf("  sensitivity=%i\n", config.sensitivity);
    printf("  deadzone=%i\n", config.deadzone);
    printf("  vibration=%i\n", config.vibration);
    printf("  ts_offset_x=%f\n", config.ts_offset_x);
    printf("  ts_offset_y=%f\n", config.ts_offset_y);
    printf("  imu_0_offset_x=%f\n", config.imu_0_offset_x);
    printf("  imu_0_offset_y=%f\n", config.imu_0_offset_y);
    printf("  imu_1_offset_x=%f\n", config.imu_1_offset_x);
    printf("  imu_1_offset_y=%f\n", config.imu_1_offset_y);
}

void config_set_profile(uint8_t profile) {
    config_nvm_t config;
    config_read(&config);
    config.profile = profile;
    config_write(&config);
}

uint8_t config_get_profile() {
    config_nvm_t config;
    config_read(&config);
    return config.profile;
}

void config_set_thumbstick_offset(float x, float y) {
    config_nvm_t config;
    config_read(&config);
    config.ts_offset_x = x,
    config.ts_offset_y = y,
    config_write(&config);
}

void config_set_imu_offset(double ax, double ay, double bx, double by) {
    config_nvm_t config;
    config_read(&config);
    config.imu_0_offset_x = ax,
    config.imu_0_offset_y = ay,
    config.imu_1_offset_x = bx,
    config.imu_1_offset_y = by,
    config_write(&config);
}

bool config_get_os_mode() {
    config_nvm_t config;
    config_read(&config);
    return config.os_mode;
}

void config_tune_update_leds() {
    config_nvm_t config;
    config_read(&config);
    if (config_tune_mode == PROC_TUNE_OS) {
        led_shape_all_off();
        led_set(LED_UP, true);
        if (config.os_mode) led_blink_mask(LED_MASK_RIGHT);
        else led_blink_mask(LED_MASK_DOWN);
    }
    if (config_tune_mode == PROC_TUNE_SENSITIVITY) {
        led_shape_all_off();
        led_set(LED_DOWN, true);
        if (config.sensitivity == 0) led_blink_mask(LED_MASK_LEFT);
        if (config.sensitivity == 1) led_blink_mask(LED_MASK_UP);
        if (config.sensitivity == 2) led_blink_mask(LED_MASK_RIGHT);
    }
    if (config_tune_mode == PROC_TUNE_DEADZONE) {
        led_shape_all_off();
        led_set(LED_LEFT, true);
        if (config.deadzone == 0) led_blink_mask(LED_MASK_DOWN);
        if (config.deadzone == 1) led_blink_mask(LED_MASK_RIGHT);
        if (config.deadzone == 2) led_blink_mask(LED_MASK_UP);
    }
    if (config_tune_mode == PROC_TUNE_VIBRATION) {
        led_shape_all_off();
        led_set(LED_RIGHT, true);
        if (config.vibration == 0) led_blink_mask(LED_MASK_DOWN);
        if (config.vibration == 1) led_blink_mask(LED_MASK_LEFT);
        if (config.vibration == 2) led_blink_mask(LED_MASK_UP);
    }
}

void config_tune_set_mode(uint8_t mode) {
    // printf("Tune: mode %i\n", mode);
    config_tune_mode = mode;
    config_tune_update_leds();
}

void config_tune(bool direction) {
    config_nvm_t config;
    config_read(&config);
    int8_t value = direction ? 1 : -1;
    if (config_tune_mode == PROC_TUNE_OS) {
        config.os_mode = direction;
        config_write(&config);
    }
    if (config_tune_mode == PROC_TUNE_SENSITIVITY) {
        config.sensitivity = limit_between(config.sensitivity + value, 0, 2);
        config_write(&config);
        imu_update_sensitivity();
    }
    if (config_tune_mode == PROC_TUNE_DEADZONE) {
        config.deadzone = limit_between(config.deadzone + value, 0, 2);
        config_write(&config);
        thumbstick_update_deadzone();
    }
    if (config_tune_mode == PROC_TUNE_VIBRATION) {
        config.vibration = limit_between(config.vibration + value, 0, 2);
        config_write(&config);
        // Update vibration somewhere.
    }
    config_tune_update_leds();
}

void config_restart(uint32_t delay) {
    watchdog_enable(delay, false);
}

void config_bootsel() {
    reset_usb_boot(0, 0);
}

void config_calibrate_execute() {
    led_shape_all_off();
    thumbstick_calibrate();
    imu_calibrate();
    led_shape_all_off();
    profile_led_lock = false;
    profile_update_leds();
}

void config_calibrate() {
    profile_led_lock = true;
    led_shape_all_off();
    led_blink_mask(LED_MASK_LEFT | LED_MASK_RIGHT);
    add_alarm_in_ms(5000, (alarm_callback_t)config_calibrate_execute, NULL, true);
}

void config_init() {
    printf("Config NVM\n");
    config_nvm_t config;
    config_read(&config);
    if (
        config.header != NVM_CONFIG_HEADER ||
        config.config_version < CFG_STRUCT_VERSION
    ) {
        printf("  config not found or outdated, writing default instead\n");
        config_write_init();
    }
    config_print();
}


