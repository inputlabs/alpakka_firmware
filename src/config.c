// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include <hardware/sync.h>
#include <hardware/watchdog.h>
#include <pico/bootrom.h>
#include <pico/unique_id.h>
#include "config.h"
#include "nvm.h"
#include "led.h"
#include "hid.h"
#include "imu.h"
#include "thumbstick.h"
#include "touch.h"
#include "profile.h"
#include "helper.h"

uint8_t config_tune_mode = 0;
uint8_t pcb_gen = 255;

void config_read(config_nvm_t* config) {
    nvm_read(XIP_BASE + NVM_CONFIG_ADDR, (uint8_t*)config, 256);
}

void config_write(config_nvm_t* config) {
    nvm_write(NVM_CONFIG_ADDR, (uint8_t*)config, 256);
}

void config_write_init() {
    // Default values when the config is created for first time.
    config_nvm_t config = {
        .header = NVM_CONFIG_HEADER,
        .config_version = CFG_STRUCT_VERSION,
        .os_mode = 0,
        .profile = 1,
        .sensitivity = 0,
        .deadzone = 0,
        .touch_threshold = 0,
        .vibration = 0,
        .ts_offset_x = 0,
        .ts_offset_y = 0,
        .imu_0_offset_x = 0,
        .imu_0_offset_y = 0,
        .imu_0_offset_z = 0,
        .imu_1_offset_x = 0,
        .imu_1_offset_y = 0,
        .imu_1_offset_z = 0
    };
    config_write(&config);
}

void config_print() {
    config_nvm_t config;
    config_read(&config);
    printf("NVM: dump\n");
    printf("  config_version=%i\n", config.config_version);
    printf("  os_mode=%i\n", config.os_mode);
    printf("  profile=%i\n", config.profile);
    printf("  sensitivity=%i\n", config.sensitivity);
    printf("  deadzone=%i\n", config.deadzone);
    printf("  touch_threshold=%i\n", config.touch_threshold);
    printf("  vibration=%i\n", config.vibration);
    printf("  ts_offset_x=%f\n", config.ts_offset_x);
    printf("  ts_offset_y=%f\n", config.ts_offset_y);
    printf("  imu_0_offset_x=%f\n", config.imu_0_offset_x);
    printf("  imu_0_offset_y=%f\n", config.imu_0_offset_y);
    printf("  imu_0_offset_z=%f\n", config.imu_0_offset_z);
    printf("  imu_1_offset_x=%f\n", config.imu_1_offset_x);
    printf("  imu_1_offset_y=%f\n", config.imu_1_offset_y);
    printf("  imu_1_offset_z=%f\n", config.imu_1_offset_z);
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

void config_set_imu_offset(double ax, double ay, double az, double bx, double by, double bz) {
    config_nvm_t config;
    config_read(&config);
    config.imu_0_offset_x = ax,
    config.imu_0_offset_y = ay,
    config.imu_0_offset_z = az,
    config.imu_1_offset_x = bx,
    config.imu_1_offset_y = by,
    config.imu_1_offset_z = bz,
    config_write(&config);
}

uint8_t config_get_os_mode() {
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
        if (config.os_mode == 0) led_blink_mask(LED_MASK_LEFT);
        if (config.os_mode == 1) led_blink_mask(LED_MASK_DOWN);
        if (config.os_mode == 2) led_blink_mask(LED_MASK_RIGHT);
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
    if (config_tune_mode == PROC_TUNE_TOUCH_THRESHOLD) {
        led_shape_all_off();
        led_set(LED_RIGHT, true);
        if (config.touch_threshold == 0) led_blink_mask(LED_MASK_DOWN);
        if (config.touch_threshold == 1) led_blink_mask(LED_MASK_DOWN + LED_MASK_LEFT);
        if (config.touch_threshold == 2) led_blink_mask(LED_MASK_LEFT);
        if (config.touch_threshold == 3) led_blink_mask(LED_MASK_LEFT + LED_MASK_UP);
        if (config.touch_threshold == 4) led_blink_mask(LED_MASK_UP);
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
        config.os_mode = limit_between(config.os_mode + value, 0, 2);
        printf("Tune: OS mode set to preset %i\n", config.os_mode);
        config_write(&config);
        profile_pending_reboot = true;
        hid_allow_communication = false;
    }
    if (config_tune_mode == PROC_TUNE_SENSITIVITY) {
        config.sensitivity = limit_between(config.sensitivity + value, 0, 2);
        config_write(&config);
        printf("Tune: Mouse sensitivity set to preset %i\n", config.sensitivity);
        imu_update_sensitivity();
    }
    if (config_tune_mode == PROC_TUNE_DEADZONE) {
        config.deadzone = limit_between(config.deadzone + value, 0, 2);
        config_write(&config);
        printf("Tune: Thumbstick deadzone set to preset %i\n", config.deadzone);
        thumbstick_update_deadzone();
    }
    if (config_tune_mode == PROC_TUNE_TOUCH_THRESHOLD) {
        config.touch_threshold = limit_between(config.touch_threshold + value, 0, 4);
        printf("Tune: Touch threshold set to preset %i\n", config.touch_threshold);
        config_write(&config);
        touch_update_threshold();
    }
    config_tune_update_leds();
}

void config_reboot() {
    watchdog_enable(100, false);  // Reboot after 100 milliseconds.
}

void config_bootsel() {
    reset_usb_boot(0, 0);
}

void config_calibrate_execute(alarm_id_t alarm) {
    cancel_alarm(alarm);
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

void config_set_pcb_gen(uint8_t gen) {
    pcb_gen = gen;
}

uint8_t config_get_pcb_gen() {
    if (pcb_gen == 255) {
        printf("ERROR: PCB gen could not be determined\n");
        sleep_ms(1000000000);
    }
    return pcb_gen;
}

void config_init() {
    char pico_id[64];
    pico_get_unique_board_id_string(pico_id, 64);
    printf("Pico ID: %s\n", pico_id);
    printf("INIT: Config\n");
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


