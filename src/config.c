// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
#include "common.h"
#include "logging.h"

// Config values.
Config config_cache;
bool config_cache_synced = true;

// Profiles.
CtrlProfile config_profile_cache[NVM_PROFILE_SLOTS];
bool config_profile_cache_synced[NVM_PROFILE_SLOTS] = {0,};

// Misc.
uint8_t config_tune_mode = 0;
uint8_t pcb_gen = 255;

// Problems.
bool problem_calibration = false;
bool problem_gyro = false;


void config_load() {
    // Load main config from NVM into the cache.
    nvm_read(NVM_CONFIG_ADDR, (uint8_t*)&config_cache, NVM_CONFIG_SIZE);
}

void config_profile_load(uint8_t index) {
    // Load a profile from NVM into the cache.
    uint32_t addr = NVM_CONFIG_ADDR + (NVM_PROFILE_SIZE * (index+1));
    nvm_read(addr, (uint8_t*)&config_profile_cache[index], NVM_PROFILE_SIZE);
    config_profile_cache_synced[index] = true;
}

Config* config_read() {
    // Access the raw config cache.
    // (Specific setting get/set are prefered than using this).
    return &config_cache;
}

CtrlProfile* config_profile_read(uint8_t index) {
    // Get a profile from cache.
    return &(config_profile_cache[index]);
}

void config_write() {
    // Write main config from cache to NVM.
    info("NVM: Config write\n");
    nvm_write(NVM_CONFIG_ADDR, (uint8_t*)&config_cache, NVM_CONFIG_SIZE);
    config_cache_synced = true;
}

void config_profile_write(uint8_t index) {
    // Write a profile from cache to NVM.
    info("NVM: Profile %i write\n", index);
    uint32_t addr = NVM_CONFIG_ADDR + (NVM_PROFILE_SIZE * (index+1));
    nvm_write(addr, (uint8_t*)&config_profile_cache[index], NVM_PROFILE_SIZE);
    config_profile_cache_synced[index] = true;
}

void config_profile_set_sync(uint8_t index, bool state) {
    // Flag a profile as synced or unsynced.
    config_profile_cache_synced[index] = state;
}

void config_sync() {
    // Sync main config.
    if (!config_cache_synced) {
        config_write();
    }
    // Sync profiles.
    for(uint8_t i=0; i<NVM_PROFILE_SLOTS; i++) {
        if (!config_profile_cache_synced[i]) {
            config_profile_write(i);
        }
    }
}

void config_write_init() {
    // Default values when the config is created for first time.
    config_cache = (Config){
        .header = NVM_CONFIG_HEADER,
        .config_version = NVM_STRUCT_VERSION,
        .profile = 1,
        .protocol = 0,
        .sens_mouse = 0,
        .sens_touch = 0,
        .deadzone = 0,
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
    config_cache.sens_mouse_values[0] = 1.0,
    config_cache.sens_mouse_values[1] = 1.5,
    config_cache.sens_mouse_values[2] = 2.0,
    config_cache.deadzone_values[0] = 0.08,
    config_cache.deadzone_values[1] = 0.12,
    config_cache.deadzone_values[2] = 0.20,
    // Touch sens values are initialized elsewhere after determining the PCB gen.
    config_write();
}

void config_delete() {
    config_cache = (Config){
        .header = 0,
        .config_version = 0,
    };
    config_write();
}

void config_print() {
    info("NVM: dump\n");
    info("  config_version=%i\n", config_cache.config_version);
    info("  protocol=%i\n", config_cache.protocol);
    info("  sens_mouse: preset=%i (%.1f, %.1f, %.1f)\n",
        config_cache.sens_mouse,
        config_cache.sens_mouse_values[0],
        config_cache.sens_mouse_values[1],
        config_cache.sens_mouse_values[2]
    );
    info("  sens_touch: preset=%i (auto, %i, %i, %i, %i) \n",
        config_cache.sens_touch,
        config_cache.sens_touch_values[1],
        config_cache.sens_touch_values[2],
        config_cache.sens_touch_values[3],
        config_cache.sens_touch_values[4]
    );
    info("  deadzone: preset=%i (%.2f, %.2f, %.2f)\n",
        config_cache.deadzone,
        config_cache.deadzone_values[0],
        config_cache.deadzone_values[1],
        config_cache.deadzone_values[2]
    );
    info("  vibration=%i\n", config_cache.vibration);
    info("  profile=%i\n", config_cache.profile);
    info("  offset_thumbstick x=%.4f y=%.4f\n",
        config_cache.offset_ts_x,
        config_cache.offset_ts_y
    );
    info("  offset_gyro_0  x=%8.2f y=%8.2f z=%8.2f\n",
        config_cache.offset_gyro_0_x,
        config_cache.offset_gyro_0_y,
        config_cache.offset_gyro_0_z
    );
    info("  offset_gyro_1  x=%8.2f y=%8.2f z=%8.2f\n",
        config_cache.offset_gyro_1_x,
        config_cache.offset_gyro_1_y,
        config_cache.offset_gyro_1_z
    );
    info("  offset_accel_0 x=%8.2f y=%8.2f z=%8.2f\n",
        config_cache.offset_accel_0_x,
        config_cache.offset_accel_0_y,
        config_cache.offset_accel_0_z
    );
    info("  offset_accel_1 x=%8.2f y=%8.2f z=%8.2f\n",
        config_cache.offset_accel_1_x,
        config_cache.offset_accel_1_y,
        config_cache.offset_accel_1_z
    );
    if (config_cache.offset_ts_x == 0 && config_cache.offset_ts_y == 0) {
        warn("The controller is not calibrated\n");
        warn("Please run calibration\n");
        config_set_problem_calibration(true);
    }
}

void config_set_profile(uint8_t profile) {
    if (profile == config_cache.profile) return;
    config_cache.profile = profile;
    config_cache_synced = false;
}

uint8_t config_get_profile() {
    return config_cache.profile;
}

void config_set_thumbstick_offset(float x, float y) {
    config_cache.offset_ts_x = x;
    config_cache.offset_ts_y = y;
    config_cache_synced = false;
}

void config_set_gyro_offset(double ax, double ay, double az, double bx, double by, double bz) {
    config_cache.offset_gyro_0_x = ax,
    config_cache.offset_gyro_0_y = ay,
    config_cache.offset_gyro_0_z = az,
    config_cache.offset_gyro_1_x = bx,
    config_cache.offset_gyro_1_y = by,
    config_cache.offset_gyro_1_z = bz,
    config_cache_synced = false;
}

void config_set_accel_offset(double ax, double ay, double az, double bx, double by, double bz) {
    config_cache.offset_accel_0_x = ax,
    config_cache.offset_accel_0_y = ay,
    config_cache.offset_accel_0_z = az,
    config_cache.offset_accel_1_x = bx,
    config_cache.offset_accel_1_y = by,
    config_cache.offset_accel_1_z = bz,
    config_cache_synced = false;
}

void config_tune_update_leds() {
    if (config_tune_mode == PROC_TUNE_OS) {
        led_static_mask(LED_UP);
        if (config_cache.protocol == 0) led_blink_mask(LED_LEFT);
        if (config_cache.protocol == 1) led_blink_mask(LED_DOWN);
        if (config_cache.protocol == 2) led_blink_mask(LED_RIGHT);
        led_set_mode(LED_MODE_BLINK);

    }
    if (config_tune_mode == PROC_TUNE_MOUSE_SENS) {
        led_static_mask(LED_DOWN);
        if (config_cache.sens_mouse == 0) led_blink_mask(LED_LEFT);
        if (config_cache.sens_mouse == 1) led_blink_mask(LED_UP);
        if (config_cache.sens_mouse == 2) led_blink_mask(LED_RIGHT);
        led_set_mode(LED_MODE_BLINK);
    }
    if (config_tune_mode == PROC_TUNE_DEADZONE) {
        led_static_mask(LED_LEFT);
        if (config_cache.deadzone == 0) led_blink_mask(LED_DOWN);
        if (config_cache.deadzone == 1) led_blink_mask(LED_RIGHT);
        if (config_cache.deadzone == 2) led_blink_mask(LED_UP);
        led_set_mode(LED_MODE_BLINK);
    }
    if (config_tune_mode == PROC_TUNE_TOUCH_SENS) {
        led_static_mask(LED_RIGHT);
        if (config_cache.sens_touch == 0) led_blink_mask(LED_DOWN);
        if (config_cache.sens_touch == 1) led_blink_mask(LED_DOWN + LED_LEFT);
        if (config_cache.sens_touch == 2) led_blink_mask(LED_LEFT);
        if (config_cache.sens_touch == 3) led_blink_mask(LED_LEFT + LED_UP);
        if (config_cache.sens_touch == 4) led_blink_mask(LED_UP);
        led_set_mode(LED_MODE_BLINK);
    }
}

void config_tune_set_mode(uint8_t mode) {
    // info("Tune: mode %i\n", mode);
    config_tune_mode = mode;
    config_tune_update_leds();
}

void config_tune(bool direction) {
    int8_t value = direction ? 1 : -1;
    if (config_tune_mode == PROC_TUNE_OS) {
        config_set_protocol(constrain(config_cache.protocol + value, 0, 2));
    }
    else if (config_tune_mode == PROC_TUNE_MOUSE_SENS) {
        config_set_mouse_sens_preset(constrain(config_cache.sens_mouse + value, 0, 2), true);
    }
    else if (config_tune_mode == PROC_TUNE_DEADZONE) {
        config_set_deadzone_preset(constrain(config_cache.deadzone + value, 0, 2), true);
    }
    else if (config_tune_mode == PROC_TUNE_TOUCH_SENS) {
        config_set_touch_sens_preset(constrain(config_cache.sens_touch + value, 0, 4), true);
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
    config_delete();
    config_reboot();
}

void config_calibrate_execute() {
    led_set_mode(LED_MODE_CYCLE);
    thumbstick_calibrate();
    imu_calibrate();
    profile_led_lock = false;
    led_set_mode(LED_MODE_IDLE);
}

void config_calibrate() {
    logging_set_onloop(false);
    info("Calibration about to start, leave the controller on a flat surface\n");
    profile_led_lock = true;
    led_static_mask(LED_NONE);
    led_blink_mask(LED_LEFT | LED_RIGHT);
    led_set_mode(LED_MODE_BLINK);
    for(uint8_t i=0; i<5; i++) {
        info("%i... ", 5-i);
        sleep_ms(1000);
    }
    info("\n");
    config_calibrate_execute();
    config_set_problem_calibration(false);
    led_set_mode(LED_MODE_IDLE);
    info("Calibration completed\n");
    logging_set_onloop(true);
}

void config_set_pcb_gen(uint8_t gen) {
    pcb_gen = gen;
    // If touch sens presets were never initialized before.
    if (config_get_touch_sens_value(1) == 0) {
        if (gen == 0) {
            uint8_t values[] = {0, 8, 5, 3, 2};
            config_set_touch_sens_values(values);
        } else {
            uint8_t values[] = {0, 40, 25, 15, 10};
            config_set_touch_sens_values(values);
        }
    }
}

uint8_t config_get_pcb_gen() {
    if (pcb_gen == 255) {
        error("PCB gen could not be determined\n");
        exit(1);
    }
    return pcb_gen;
}

uint8_t config_get_protocol() {
    return config_cache.protocol;
}

uint8_t config_get_touch_sens_preset() {
    return config_cache.sens_touch;
}

uint8_t config_get_mouse_sens_preset() {
    return config_cache.sens_mouse;
}

uint8_t config_get_deadzone_preset() {
    return config_cache.deadzone;
}

void config_set_protocol(uint8_t preset) {
    if (preset == config_cache.protocol) return;
    config_cache.protocol = preset;
    config_write();
    profile_pending_reboot = true;
    hid_allow_communication = false;
    info("Config: Protocol preset %i\n", preset);
}

void config_set_touch_sens_preset(uint8_t preset, bool notify_webusb) {
    config_cache.sens_touch = preset;
    touch_update_threshold();
    if (notify_webusb) webusb_set_pending_config_share(SENS_TOUCH);
    info("Config: Touch sensitivity preset %i\n", preset);
}

void config_set_mouse_sens_preset(uint8_t preset, bool notify_webusb) {
    config_cache.sens_mouse = preset;
    gyro_update_sensitivity();
    if (notify_webusb) webusb_set_pending_config_share(SENS_MOUSE);
    info("Config: Mouse sensitivity preset %i\n", preset);
}

void config_set_deadzone_preset(uint8_t preset, bool notify_webusb) {
    config_cache.deadzone = preset;
    thumbstick_update_deadzone();
    if (notify_webusb) webusb_set_pending_config_share(DEADZONE);
    info("Config: Deadzone preset %i\n", preset);
}

uint8_t config_get_touch_sens_value(uint8_t index) {
    return config_cache.sens_touch_values[index];
}

double config_get_mouse_sens_value(uint8_t index) {
    return config_cache.sens_mouse_values[index];
}

float config_get_deadzone_value(uint8_t index) {
    return config_cache.deadzone_values[index];
}

void config_set_touch_sens_values(uint8_t* values) {
    config_cache.sens_touch_values[1] = values[1];
    config_cache.sens_touch_values[2] = values[2];
    config_cache.sens_touch_values[3] = values[3];
    config_cache.sens_touch_values[4] = values[4];
    config_cache_synced = false;
}

void config_set_mouse_sens_values(double* values) {
    config_cache.sens_mouse_values[0] = values[0];
    config_cache.sens_mouse_values[1] = values[1];
    config_cache.sens_mouse_values[2] = values[2];
    config_cache_synced = false;
}

void config_set_deadzone_values(float* values) {
    config_cache.deadzone_values[0] = values[0];
    config_cache.deadzone_values[1] = values[1];
    config_cache.deadzone_values[2] = values[2];
    config_cache_synced = false;
}

void config_set_problem_calibration(bool state) {
    problem_calibration = state;
    led_show();
}

void config_set_problem_gyro(bool state) {
    problem_gyro = state;
    led_show();
}

void config_ignore_problems() {
    if (!config_problems_are_pending()) return;
    warn("User requested to ignore problems\n");
    problem_calibration = false;
    problem_gyro = false;
    led_show();
}

bool config_problems_are_pending() {
    return problem_calibration || problem_gyro;
}

void config_init_profiles_from_defaults() {
    warn("Loading profiles from defaults\n");
    config_profile_default_home(           &(config_profile_cache[PROFILE_HOME]));
    config_profile_default_fps_fusion(     &(config_profile_cache[PROFILE_FPS_FUSION]));
    config_profile_default_racing(         &(config_profile_cache[PROFILE_RACING]));
    config_profile_default_console(        &(config_profile_cache[PROFILE_CONSOLE]));
    config_profile_default_desktop(        &(config_profile_cache[PROFILE_DESKTOP]));
    config_profile_default_fps_wasd(       &(config_profile_cache[PROFILE_FPS_WASD]));
    config_profile_default_flight(         &(config_profile_cache[PROFILE_FLIGHT]));
    config_profile_default_console_legacy( &(config_profile_cache[PROFILE_CONSOLE_LEGACY]));
    config_profile_default_rts(            &(config_profile_cache[PROFILE_RTS]));
    config_profile_default_custom(         &(config_profile_cache[PROFILE_CUSTOM_1]));
    config_profile_default_custom(         &(config_profile_cache[PROFILE_CUSTOM_2]));
    config_profile_default_custom(         &(config_profile_cache[PROFILE_CUSTOM_3]));
    config_profile_default_custom(         &(config_profile_cache[PROFILE_CUSTOM_4]));
    config_profile_default_console_legacy( &(config_profile_cache[PROFILE_HOME_GAMEPAD]));
    for(uint8_t i=0; i<NVM_PROFILE_SLOTS; i++) {
        config_profile_write(i);
    }
}

void config_init_profiles_from_nvm() {
    info("NVM: Loading profiles\n");
    for(uint8_t i=0; i<NVM_PROFILE_SLOTS; i++) {
        config_profile_load(i);
    }
}

void config_init() {
    char pico_id[64];
    pico_get_unique_board_id_string(pico_id, 64);
    info("Pico UID: %s\n", pico_id);
    info("INIT: Config\n");
    config_load();
    if (
        config_cache.header != NVM_CONFIG_HEADER ||
        config_cache.config_version != NVM_STRUCT_VERSION
    ) {
        warn("NVM config not found or incompatible, writing default instead\n");
        config_write_init();
        config_init_profiles_from_defaults();
    } else {
        config_init_profiles_from_nvm();
    }
    config_print();
}


