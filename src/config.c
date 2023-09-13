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
#include "webusb.h"
#include "helper.h"
#include "logging.h"

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
        .offset_ts_x = 0,
        .offset_ts_y = 0,
        .offset_gyro_0_x = 0,
        .offset_gyro_0_y = 0,
        .offset_gyro_0_z = 0,
        .offset_gyro_1_x = 0,
        .offset_gyro_1_y = 0,
        .offset_gyro_1_z = 0,
        .offset_accel_0_x = 0,
        .offset_accel_0_y = 0,
        .offset_accel_0_z = 0,
        .offset_accel_1_x = 0,
        .offset_accel_1_y = 0,
        .offset_accel_1_z = 0
    };
    config_write(&config);
}

void config_print() {
    config_nvm_t config;
    config_read(&config);
    info("NVM: dump\n");
    info("  config_version=%i\n", config.config_version);
    info("  os_mode=%i\n", config.os_mode);
    info("  profile=%i\n", config.profile);
    info("  sensitivity=%i\n", config.sensitivity);
    info("  deadzone=%i\n", config.deadzone);
    info("  touch_threshold=%i\n", config.touch_threshold);
    info("  vibration=%i\n", config.vibration);
    info("  offset_thumbstick x=%.4f y=%.4f\n",
        config.offset_ts_x,
        config.offset_ts_y
    );
    info("  offset_gyro_0  x=%8.2f y=%8.2f z=%8.2f\n",
        config.offset_gyro_0_x,
        config.offset_gyro_0_y,
        config.offset_gyro_0_z
    );
    info("  offset_gyro_1  x=%8.2f y=%8.2f z=%8.2f\n",
        config.offset_gyro_1_x,
        config.offset_gyro_1_y,
        config.offset_gyro_1_z
    );
    info("  offset_accel_0 x=%8.2f y=%8.2f z=%8.2f\n",
        config.offset_accel_0_x,
        config.offset_accel_0_y,
        config.offset_accel_0_z
    );
    info("  offset_accel_1 x=%8.2f y=%8.2f z=%8.2f\n",
        config.offset_accel_1_x,
        config.offset_accel_1_y,
        config.offset_accel_1_z
    );
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
    config.offset_ts_x = x,
    config.offset_ts_y = y,
    config_write(&config);
}

void config_set_gyro_offset(double ax, double ay, double az, double bx, double by, double bz) {
    config_nvm_t config;
    config_read(&config);
    config.offset_gyro_0_x = ax,
    config.offset_gyro_0_y = ay,
    config.offset_gyro_0_z = az,
    config.offset_gyro_1_x = bx,
    config.offset_gyro_1_y = by,
    config.offset_gyro_1_z = bz,
    config_write(&config);
}

void config_set_accel_offset(double ax, double ay, double az, double bx, double by, double bz) {
    config_nvm_t config;
    config_read(&config);
    config.offset_accel_0_x = ax,
    config.offset_accel_0_y = ay,
    config.offset_accel_0_z = az,
    config.offset_accel_1_x = bx,
    config.offset_accel_1_y = by,
    config.offset_accel_1_z = bz,
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
    // info("Tune: mode %i\n", mode);
    config_tune_mode = mode;
    config_tune_update_leds();
}

void config_tune(bool direction) {
    config_nvm_t config;
    config_read(&config);
    int8_t value = direction ? 1 : -1;
    if (config_tune_mode == PROC_TUNE_OS) {
        config_set_protocol(constrain(config.os_mode + value, 0, 2));
    }
    else if (config_tune_mode == PROC_TUNE_SENSITIVITY) {
        config_set_mouse_sens(constrain(config.sensitivity + value, 0, 2));
    }
    else if (config_tune_mode == PROC_TUNE_DEADZONE) {
        config_set_deadzone(constrain(config.deadzone + value, 0, 2));
    }
    else if (config_tune_mode == PROC_TUNE_TOUCH_THRESHOLD) {
        config_set_touch_sens(constrain(config.touch_threshold + value, 0, 4));
    }
    config_tune_update_leds();
}

void config_reboot() {
    watchdog_enable(1, false);  // Reboot after 1 millisecond.
    sleep_ms(10);
}

void config_bootsel() {
    reset_usb_boot(0, 0);
}

void config_factory() {
    info("NVM: Reset to factory defaults\n");
    config_write_init();
    config_print();
    config_reboot();
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
    logging_set_onloop(false);
    info("Calibration about to start, leave the controller on a flat surface\n");
    profile_led_lock = true;
    led_shape_all_off();
    led_blink_mask(LED_MASK_LEFT | LED_MASK_RIGHT);
    for(uint8_t i=0; i<5; i++) {
        info("%i... ", 5-i);
        sleep_ms(1000);
    }
    info("\n");
    config_calibrate_execute();
    info("Calibration completed\n");
    logging_set_onloop(true);
}

void config_set_pcb_gen(uint8_t gen) {
    pcb_gen = gen;
}

uint8_t config_get_pcb_gen() {
    if (pcb_gen == 255) {
        info("ERROR: PCB gen could not be determined\n");
        sleep_ms(1000000000);
    }
    return pcb_gen;
}

uint8_t config_get_protocol() {
    config_nvm_t config;
    config_read(&config);
    return config.os_mode;
}

void config_set_protocol(uint8_t preset) {
    config_nvm_t config;
    config_read(&config);
    if (preset == config.os_mode) return;
    config.os_mode = preset;
    config_write(&config);
    profile_pending_reboot = true;
    hid_allow_communication = false;
    info("Config: Protocol preset %i\n", preset);
}

uint8_t config_get_touch_sens() {
    config_nvm_t config;
    config_read(&config);
    return config.touch_threshold;
}

void config_set_touch_sens(uint8_t preset) {
    config_nvm_t config;
    config_read(&config);
    config.touch_threshold = preset;
    config_write(&config);
    touch_update_threshold();
    webusb_set_pending_proc_refresh(true);
    info("Config: Touch sensitivity preset %i\n", preset);
}

uint8_t config_get_mouse_sens() {
    config_nvm_t config;
    config_read(&config);
    return config.sensitivity;
}

void config_set_mouse_sens(uint8_t preset) {
    config_nvm_t config;
    config_read(&config);
    config.sensitivity = preset;
    config_write(&config);
    gyro_update_sensitivity();
    webusb_set_pending_proc_refresh(true);
    info("Config: Mouse sensitivity preset %i\n", preset);
}

uint8_t config_get_deadzone() {
    config_nvm_t config;
    config_read(&config);
    return config.deadzone;
}

void config_set_deadzone(uint8_t preset) {
    config_nvm_t config;
    config_read(&config);
    config.deadzone = preset;
    config_write(&config);
    thumbstick_update_deadzone();
    webusb_set_pending_proc_refresh(true);
    info("Config: Deadzone preset %i\n", preset);
}

void config_init() {
    char pico_id[64];
    pico_get_unique_board_id_string(pico_id, 64);
    info("Pico UID: %s\n", pico_id);
    info("INIT: Config\n");
    config_nvm_t config;
    config_read(&config);
    if (
        config.header != NVM_CONFIG_HEADER ||
        config.config_version != CFG_STRUCT_VERSION
    ) {
        info("  config not found or incompatible, writing default instead\n");
        config_write_init();
    }
    config_print();
}


