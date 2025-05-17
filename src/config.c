// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hardware/sync.h>
#include <pico/unique_id.h>
#include "config.h"
#include "nvm.h"
#include "led.h"
#include "pin.h"
#include "hid.h"
#include "imu.h"
#include "thumbstick.h"
#include "touch.h"
#include "profile.h"
#include "webusb.h"
#include "common.h"
#include "logging.h"
#include "power.h"

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
bool problem_battery_low = false;


void config_load() {
    // Load main config from NVM into the cache.
    nvm_read(NVM_CONFIG_ADDR, (uint8_t*)&config_cache, NVM_CONFIG_SIZE);
}

void config_profile_clear_cache(uint8_t index) {
    debug("Config: Clear profile %i cache\n", index);
    memset(&config_profile_cache[index], 0, sizeof(CtrlProfile));
}

void config_profile_load(uint8_t index) {
    debug("Config: Profile %i load from NVM\n", index);
    // Nuke cache.
    config_profile_clear_cache(index);
    // Load NVM into cache.
    uint32_t addr = NVM_CONFIG_ADDR + (NVM_PROFILE_SIZE * (index+1));
    nvm_read(addr, (uint8_t*)&config_profile_cache[index], sizeof(CtrlProfile));
    // Check if loaded profile is valid, otherwise load default.
    CtrlProfileMeta meta = config_profile_cache[index].sections[SECTION_META].meta;
    if (meta.control_byte != NVM_CONTROL_BYTE) {
        debug("Config: Profile %i not found\n", index);
        config_profile_default(index, index);
    }
    uint32_t version = (
        (meta.version_major * 1000000) +
        (meta.version_minor * 1000) +
        (meta.version_patch)
    );
    if (version < NVM_PROFILE_VERSION) {
        debug("Config: Profile %i incompatible version (%lu)\n", index, version);
        config_profile_default(index, index);
    }
    // Tag as synced.
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
    // Do not check in every cycle.
    static uint16_t i = 0;
    i++;
    if (i != NVM_SYNC_FREQUENCY) return;
    else i = 0;
    // Sync main config.
    if (!config_cache_synced) {
        config_write();
    }
    // Sync profiles.
    #ifdef DEVICE_IS_ALPAKKA
        for(uint8_t i=0; i<NVM_PROFILE_SLOTS; i++) {
            if (!config_profile_cache_synced[i]) {
                config_profile_write(i);
            }
        }
    #endif
}

void config_write_init() {
    // Default values when the config is created for first time.
    config_cache = (Config){
        .header = NVM_CONTROL_BYTE,
        .config_version = NVM_CONFIG_VERSION,
        .profile = 1,
        .protocol = 0,
        .sens_mouse = 0,
        .sens_touch = 1,
        .deadzone = 1,
        .vibration = 0,
        .offset_ts_lx = 0,
        .offset_ts_ly = 0,
        .offset_ts_rx = 0,
        .offset_ts_ry = 0,
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
        .offset_accel_1_z = 0,
        .log_level = 0,
        .log_mask = 0,
        .long_calibration = 0,
        .swap_gyros = 0,
        .touch_invert_polarity = 0,
    };
    config_cache.sens_mouse_values[0] = 1.0,
    config_cache.sens_mouse_values[1] = 1.5,
    config_cache.sens_mouse_values[2] = 2.0,
    config_cache.deadzone_values[0] = 0.08,
    config_cache.deadzone_values[1] = 0.12,
    config_cache.deadzone_values[2] = 0.20,
    config_cache.sens_touch_values[0] = -1;  // Auto preset 1.
    config_cache.sens_touch_values[1] = -2;  // Auto preset 2.
    config_cache.sens_touch_values[2] = -3;  // Auto preset 3.
    config_cache.sens_touch_values[3] = 120;  // 12.0
    config_cache.sens_touch_values[4] = 60;  // 6.0
    config_write();
}

void config_delete() {
    debug("Config: Delete config header sector\n");
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
    info("  sens_touch: preset=%i (%i, %i, %i, %.1f, %.1f) \n",
        config_cache.sens_touch,
        config_cache.sens_touch_values[0],
        config_cache.sens_touch_values[1],
        config_cache.sens_touch_values[2],
        config_cache.sens_touch_values[3] / 10.0,
        config_cache.sens_touch_values[4] / 10.0
    );
    info("  deadzone: preset=%i (%.2f, %.2f, %.2f)\n",
        config_cache.deadzone,
        config_cache.deadzone_values[0],
        config_cache.deadzone_values[1],
        config_cache.deadzone_values[2]
    );
    info("  vibration=%i\n", config_cache.vibration);
    info("  profile=%i\n", config_cache.profile);
    info("  log_modes level=%i mask=%i\n", config_cache.log_level, config_cache.log_mask);
    info("  long_calibration=%i\n", config_cache.long_calibration);
    info("  swap_gyros=%i\n", config_cache.swap_gyros);
    info("  touch_invert_polarity=%i\n", config_cache.touch_invert_polarity);
    info("  offset_thumbstick_0 x=%.4f y=%.4f\n",
        config_cache.offset_ts_lx,
        config_cache.offset_ts_ly
    );
    info("  offset_thumbstick_1 x=%.4f y=%.4f\n",
        config_cache.offset_ts_rx,
        config_cache.offset_ts_ry
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
}

void config_print_minimal() {
    info("NVM: dump\n");
    info("  config_version=%i\n", config_cache.config_version);
    info("  protocol=%i\n", config_cache.protocol);
    info("  log_modes level=%i mask=%i\n", config_cache.log_level, config_cache.log_mask);
}

void config_set_profile(uint8_t profile) {
    if (profile == config_cache.profile) return;
    config_cache.profile = profile;
    config_cache_synced = false;
}

uint8_t config_get_profile() {
    return config_cache.profile;
}

void config_set_thumbstick_offset(float lx, float ly, float rx, float ry) {
    config_cache.offset_ts_lx = lx;
    config_cache.offset_ts_ly = ly;
    config_cache.offset_ts_rx = rx;
    config_cache.offset_ts_ry = ry;
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
        if (config_cache.sens_touch == 1) led_blink_mask(LED_LEFT);
        if (config_cache.sens_touch == 2) led_blink_mask(LED_UP);
        if (config_cache.sens_touch == 3) led_blink_mask(LED_LEFT + LED_DOWN);
        if (config_cache.sens_touch == 4) led_blink_mask(LED_LEFT + LED_UP);
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
    config_cache_synced = false;
    config_tune_update_leds();
}

void config_reset_factory() {
    info("NVM: Reset to factory defaults\n");
    config_profile_default_all();
    config_delete();
    power_restart();
}

void config_reset_config() {
    info("NVM: Reset config\n");
    config_delete();
    power_restart();
}

void config_reset_profiles() {
    info("NVM: Reset profiles\n");
    config_profile_default_all();
    power_restart();
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
    touch_load_from_config();
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

void config_set_log_level(LogLevel log_level) {
    info("Config: log_level=%i\n", log_level);
    config_cache.log_level = log_level;
    config_cache_synced = false;
}

void config_set_log_mask(LogMask log_mask) {
    info("Config: log_mask=%i\n", log_mask);
    config_cache.log_mask = log_mask;
    config_cache_synced = false;
}

void config_set_long_calibration(bool value) {
    info("Config: long_calibration=%i\n", value);
    config_cache.long_calibration = value;
    config_cache_synced = false;
}

void config_set_swap_gyros(bool value) {
    info("Config: swap_gyros=%i\n", value);
    config_cache.swap_gyros = value;
    config_cache_synced = false;
    imu_init();
}

void config_set_touch_invert_polarity(bool value) {
    info("Config: touch_invert_polarity=%i\n", value);
    config_cache.touch_invert_polarity = value;
    config_cache_synced = false;
    touch_load_from_config();
}

void config_set_gyro_user_offset(int8_t x, int8_t y, int8_t z) {
    float f = 0.01;
    info("Config: offset_gyro_user x=%.2f y=%.2f z=%.2f\n", x*f, y*f, z*f);
    config_cache.offset_gyro_user_x = x;
    config_cache.offset_gyro_user_y = y;
    config_cache.offset_gyro_user_z = z;
    config_cache_synced = false;
    imu_load_calibration();
}

void config_set_problem_calibration(bool state) {
    problem_calibration = state;
    led_show();
}

void config_set_problem_gyro(bool state) {
    problem_gyro = state;
    led_show();
}

void config_set_problem_battery_low(bool state) {
    if (state == problem_battery_low) return;
    problem_battery_low = state;
    led_show();
}

void config_ignore_problems() {
    if (!config_problems_are_pending()) return;
    warn("User requested to ignore problems\n");
    problem_calibration = false;
    problem_gyro = false;
    problem_battery_low = false;
    led_show();
}

bool config_problems_are_pending() {
    return (
        problem_calibration ||
        problem_gyro ||
        problem_battery_low
    );
}

void config_alert_if_not_calibrated() {
    if (config_cache.offset_ts_lx == 0 && config_cache.offset_ts_ly == 0) {
        warn("The controller is not calibrated\n");
        warn("Please run calibration\n");
        config_set_problem_calibration(true);
    }
}

void config_profile_default(uint8_t indexTo, int8_t indexFrom) {
    info("Config: Profile %i init from default %i\n", indexTo, indexFrom);
    config_profile_clear_cache(indexTo);
    if (indexFrom ==  0) config_profile_default_home(           &(config_profile_cache[indexTo]));
    if (indexFrom ==  1) config_profile_default_fps_fusion(     &(config_profile_cache[indexTo]));
    if (indexFrom ==  2) config_profile_default_racing(         &(config_profile_cache[indexTo]));
    if (indexFrom ==  3) config_profile_default_console(        &(config_profile_cache[indexTo]));
    if (indexFrom ==  4) config_profile_default_desktop(        &(config_profile_cache[indexTo]));
    if (indexFrom ==  5) config_profile_default_fps_wasd(       &(config_profile_cache[indexTo]));
    if (indexFrom ==  6) config_profile_default_flight(         &(config_profile_cache[indexTo]));
    if (indexFrom ==  7) config_profile_default_console_legacy( &(config_profile_cache[indexTo]));
    if (indexFrom ==  8) config_profile_default_rts(            &(config_profile_cache[indexTo]));
    if (indexFrom ==  9) config_profile_default_custom(         &(config_profile_cache[indexTo]));
    if (indexFrom == 10) config_profile_default_custom(         &(config_profile_cache[indexTo]));
    if (indexFrom == 11) config_profile_default_custom(         &(config_profile_cache[indexTo]));
    if (indexFrom == 12) config_profile_default_custom(         &(config_profile_cache[indexTo]));
    if (indexFrom == 13) config_profile_default_console_legacy( &(config_profile_cache[indexTo]));
    // Add number to the name of the default custom profiles.
    if (indexTo >= 9 && indexTo<=12) {
        char *name = config_profile_cache[indexTo].sections[SECTION_META].meta.name;
        char custom_name[9];  // Custom=6 +space +digit +nullterm.
        snprintf(custom_name, 9, "Custom %i", indexTo-8);
        memcpy(name, custom_name, sizeof(custom_name));
    }
    // Save in NVM.
    config_profile_write(indexTo);
}

void config_profile_default_all() {
    debug("Config: Init all profiles from defaults\n");
    for(uint8_t i=0; i<NVM_PROFILE_SLOTS; i++) {
        config_profile_default(i, i);
    }
}

void config_profile_overwrite(uint8_t indexTo, int8_t indexFrom) {
    debug("Config: Profile overwrite %i -> %i\n", indexFrom, indexTo);
    // Remember name.
    char name[24];
    CtrlProfileMeta *meta = &(config_profile_cache[indexTo]).sections[SECTION_META].meta;
    memcpy(name, meta->name, 24);
    // From default.
    if (indexFrom < 0) {
        config_profile_default(indexTo, -indexFrom);
    }
    // From other profile slot.
    if (indexFrom > 0) {
        memcpy(
            &config_profile_cache[indexTo],
            &config_profile_cache[indexFrom],
            sizeof(CtrlProfile)
        );
    }
    // Restore name.
    memcpy(meta->name, name, 24);
}

void config_init_profiles_from_nvm() {
    info("NVM: Loading profiles\n");
    for(uint8_t i=0; i<NVM_PROFILE_SLOTS; i++) {
        config_profile_load(i);
    }
}

void config_init() {
    char board_id[64];
    pico_get_unique_board_id_string(board_id, 64);
    info("Board UID: %s\n", board_id);
    info("INIT: Config\n");
    config_load();
    if (
        config_cache.header != NVM_CONTROL_BYTE ||
        config_cache.config_version != NVM_CONFIG_VERSION
    ) {
        warn("NVM config not found or incompatible, writing default instead\n");
        config_write_init();
    }
    #ifdef DEVICE_IS_ALPAKKA
        config_init_profiles_from_nvm();
        config_print();
        config_alert_if_not_calibrated();
    #else
        config_print_minimal();
    #endif
    logging_load_from_config();
}


