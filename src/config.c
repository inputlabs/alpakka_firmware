// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include <stdlib.h>
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
        .config_version = NVM_STRUCT_VERSION,
        .protocol = 0,
        .sens_mouse = 0,
        .sens_touch = 0,
        .deadzone = 0,
        .vibration = 0,
        .sens_mouse_value_0 = 1.0,
        .sens_mouse_value_1 = 1.5,
        .sens_mouse_value_2 = 2.0,
        // See https://github.com/inputlabs/alpakka_pcb/blob/main/generations.md
        .sens_touch_value_1 = config_get_pcb_gen() == 0 ? 8 : 40,
        .sens_touch_value_2 = config_get_pcb_gen() == 0 ? 5 : 25,
        .sens_touch_value_3 = config_get_pcb_gen() == 0 ? 3 : 15,
        .sens_touch_value_4 = config_get_pcb_gen() == 0 ? 2 : 10,
        .deadzone_value_0 = 0.07,
        .deadzone_value_1 = 0.10,
        .deadzone_value_2 = 0.15,
        .profile = 1,
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
    info("  protocol=%i\n", config.protocol);
    info("  sens_mouse: preset=%i (%.1f, %.1f, %.1f)\n",
        config.sens_mouse,
        config.sens_mouse_value_0,
        config.sens_mouse_value_1,
        config.sens_mouse_value_2
    );
    info("  sens_touch: preset=%i (auto, %i, %i, %i, %i) \n",
        config.sens_touch,
        config.sens_touch_value_1,
        config.sens_touch_value_2,
        config.sens_touch_value_3,
        config.sens_touch_value_4
    );
    info("  deadzone: preset=%i (%.2f, %.2f, %.2f)\n",
        config.deadzone,
        config.deadzone_value_0,
        config.deadzone_value_1,
        config.deadzone_value_2
    );
    info("  vibration=%i\n", config.vibration);
    info("  profile=%i\n", config.profile);
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

void config_tune_update_leds() {
    config_nvm_t config;
    config_read(&config);
    if (config_tune_mode == PROC_TUNE_OS) {
        led_shape_all_off();
        led_set(LED_UP, true);
        if (config.protocol == 0) led_blink_mask(LED_MASK_LEFT);
        if (config.protocol == 1) led_blink_mask(LED_MASK_DOWN);
        if (config.protocol == 2) led_blink_mask(LED_MASK_RIGHT);
    }
    if (config_tune_mode == PROC_TUNE_SENSITIVITY) {
        led_shape_all_off();
        led_set(LED_DOWN, true);
        if (config.sens_mouse == 0) led_blink_mask(LED_MASK_LEFT);
        if (config.sens_mouse == 1) led_blink_mask(LED_MASK_UP);
        if (config.sens_mouse == 2) led_blink_mask(LED_MASK_RIGHT);
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
        if (config.sens_touch == 0) led_blink_mask(LED_MASK_DOWN);
        if (config.sens_touch == 1) led_blink_mask(LED_MASK_DOWN + LED_MASK_LEFT);
        if (config.sens_touch == 2) led_blink_mask(LED_MASK_LEFT);
        if (config.sens_touch == 3) led_blink_mask(LED_MASK_LEFT + LED_MASK_UP);
        if (config.sens_touch == 4) led_blink_mask(LED_MASK_UP);
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
        config_set_protocol(constrain(config.protocol + value, 0, 2));
    }
    else if (config_tune_mode == PROC_TUNE_SENSITIVITY) {
        config_set_mouse_sens_preset(constrain(config.sens_mouse + value, 0, 2), true);
    }
    else if (config_tune_mode == PROC_TUNE_DEADZONE) {
        config_set_deadzone_preset(constrain(config.deadzone + value, 0, 2), true);
    }
    else if (config_tune_mode == PROC_TUNE_TOUCH_THRESHOLD) {
        config_set_touch_sens_preset(constrain(config.sens_touch + value, 0, 4), true);
    }
    config_tune_update_leds();
}

void config_reboot() {
    watchdog_enable(1, false);  // Reboot after 1 millisecond.
    sleep_ms(10);  // Stall the exexution to avoid resetting the timer.
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
        error("PCB gen could not be determined\n");
        exit(1);
    }
    return pcb_gen;
}

uint8_t config_get_protocol() {
    config_nvm_t config;
    config_read(&config);
    return config.protocol;
}

void config_set_protocol(uint8_t preset) {
    config_nvm_t config;
    config_read(&config);
    if (preset == config.protocol) return;
    config.protocol = preset;
    config_write(&config);
    profile_pending_reboot = true;
    hid_allow_communication = false;
    info("Config: Protocol preset %i\n", preset);
}

uint8_t config_get_touch_sens_preset() {
    config_nvm_t config;
    config_read(&config);
    return config.sens_touch;
}

void config_set_touch_sens_preset(uint8_t preset, bool notify_webusb) {
    config_nvm_t config;
    config_read(&config);
    config.sens_touch = preset;
    config_write(&config);
    touch_update_threshold();
    if (notify_webusb) webusb_set_pending_config_share(SENS_TOUCH);
    info("Config: Touch sensitivity preset %i\n", preset);
}

uint8_t config_get_touch_sens_value() {
    config_nvm_t config;
    config_read(&config);
    if (config.sens_touch == 1) return config.sens_touch_value_1;
    if (config.sens_touch == 2) return config.sens_touch_value_2;
    if (config.sens_touch == 3) return config.sens_touch_value_3;
    if (config.sens_touch == 4) return config.sens_touch_value_4;
}

uint8_t config_get_mouse_sens_preset() {
    config_nvm_t config;
    config_read(&config);
    return config.sens_mouse;
}

void config_set_mouse_sens_preset(uint8_t preset, bool notify_webusb) {
    config_nvm_t config;
    config_read(&config);
    config.sens_mouse = preset;
    config_write(&config);
    gyro_update_sensitivity();
    if (notify_webusb) webusb_set_pending_config_share(SENS_MOUSE);
    info("Config: Mouse sensitivity preset %i\n", preset);
}

double config_get_mouse_sens_value() {
    config_nvm_t config;
    config_read(&config);
    if (config.sens_mouse == 0) return config.sens_mouse_value_0;
    if (config.sens_mouse == 1) return config.sens_mouse_value_1;
    if (config.sens_mouse == 2) return config.sens_mouse_value_2;
}

uint8_t config_get_deadzone_preset() {
    config_nvm_t config;
    config_read(&config);
    return config.deadzone;
}

void config_set_deadzone_preset(uint8_t preset, bool notify_webusb) {
    config_nvm_t config;
    config_read(&config);
    config.deadzone = preset;
    config_write(&config);
    thumbstick_update_deadzone();
    if (notify_webusb) webusb_set_pending_config_share(DEADZONE);
    info("Config: Deadzone preset %i\n", preset);
}

float config_get_deadzone_value() {
    config_nvm_t config;
    config_read(&config);
    if (config.deadzone == 0) return config.deadzone_value_0;
    if (config.deadzone == 1) return config.deadzone_value_1;
    if (config.deadzone == 2) return config.deadzone_value_2;
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
        config.config_version != NVM_STRUCT_VERSION
    ) {
        warn("NVM config not found or incompatible, writing default instead\n");
        config_write_init();
    }
    config_print();
}


