// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#pragma once
#include <stdint.h>
#include <math.h>
#include <stdbool.h>
#include "ctrl.h"
#include "logging.h"

#define NVM_CONTROL_BYTE 0b01010101
#define NVM_CONFIG_ADDR 0x001D0000
#define NVM_CONFIG_SIZE 256
#define NVM_CONFIG_VERSION 97000
#define NVM_PROFILE_VERSION 96000
#define NVM_PROFILE_SIZE 4096
#define NVM_PROFILE_SLOTS 14

#define PROTOCOL_XINPUT_WIN 0
#define PROTOCOL_XINPUT_UNIX 1
#define PROTOCOL_GENERIC 2

#define CFG_LED_BRIGHTNESS 0.2

#define CFG_TICK_FREQUENCY 250  // Hz.
#define CFG_TICK_INTERVAL  (1000 / CFG_TICK_FREQUENCY)
#define CFG_IMU_TICK_SAMPLES 128  // Multi-sampling per pooling cycle.
#define CFG_HID_REPORT_PRIORITY_RATIO 8

#define NVM_SYNC_FREQUENCY  (CFG_TICK_FREQUENCY / 2)

#define CFG_CALIBRATION_SAMPLES_THUMBSTICK 100000  // Samples.
#define CFG_CALIBRATION_SAMPLES_GYRO 200000  // Samples.
#define CFG_CALIBRATION_SAMPLES_ACCEL 100000  // Samples.
#define CFG_CALIBRATION_LONG_FACTOR 8

#define CFG_GYRO_SENSITIVITY  pow(2, -9) * 1.45
#define CFG_GYRO_SENSITIVITY_X  CFG_GYRO_SENSITIVITY * 1
#define CFG_GYRO_SENSITIVITY_Y  CFG_GYRO_SENSITIVITY * 1
#define CFG_GYRO_SENSITIVITY_Z  CFG_GYRO_SENSITIVITY * 1
#define CFG_MOUSE_WHEEL_DEBOUNCE 1000
#define CFG_ACCEL_CORRECTION_SMOOTH 50  // Number of averaged samples for the correction vector.
#define CFG_ACCEL_CORRECTION_RATE 0.0007  // How fast the correction is applied.

#define CFG_PRESS_DEBOUNCE 50  // Milliseconds.
#define CFG_HOLD_TIME 200  // Milliseconds.
#define CFG_HOLD_LONG_TIME 2000  // Milliseconds.
#define CFG_DOUBLE_PRESS_TIME 300  // Milliseconds.

#define CFG_THUMBSTICK_SATURATION 1.6
#define CFG_THUMBSTICK_INNER_RADIUS 0.75
#define CFG_THUMBSTICK_ADDITIONAL_DEADZONE_FOR_BUTTONS 0.05

#define CFG_DHAT_DEBOUNCE_TIME 100  // Milliseconds.

typedef struct __packed _Config {
    uint8_t header;
    uint32_t config_version;
    uint8_t profile;
    int8_t protocol;
    int8_t sens_mouse;
    int8_t sens_touch;
    int8_t deadzone;
    int8_t vibration;
    double sens_mouse_values[3];
    int8_t sens_touch_values[5];
    float deadzone_values[3];
    float offset_ts_x;
    float offset_ts_y;
    double offset_gyro_0_x;
    double offset_gyro_0_y;
    double offset_gyro_0_z;
    double offset_gyro_1_x;
    double offset_gyro_1_y;
    double offset_gyro_1_z;
    double offset_accel_0_x;
    double offset_accel_0_y;
    double offset_accel_0_z;
    double offset_accel_1_x;
    double offset_accel_1_y;
    double offset_accel_1_z;
    uint8_t log_level;
    uint8_t log_mask;
    bool long_calibration;
    bool swap_gyros;
    bool touch_invert_polarity;
    uint8_t padding[256]; // Guarantee block is at least 256 bytes or more.
} Config;

void config_init();
void config_sync();
Config* config_read();
void config_delete();

void config_set_thumbstick_offset(float x, float y);
void config_set_gyro_offset(double ax, double ay, double az, double bx, double by, double bz);
void config_set_accel_offset(double ax, double ay, double az, double bx, double by, double bz);
uint8_t config_get_protocol();
void config_tune_set_mode(uint8_t mode);
void config_tune(bool direction);
void config_calibrate();
void config_reboot();
void config_bootsel();
void config_reset_config();
void config_reset_profiles();
void config_reset_factory();
void config_write_init();
void config_print();
void config_set_pcb_gen(uint8_t gen);
uint8_t config_get_pcb_gen();

uint8_t config_get_protocol();
uint8_t config_get_touch_sens_preset();
uint8_t config_get_mouse_sens_preset();
uint8_t config_get_deadzone_preset();

void config_set_protocol(uint8_t preset);
void config_set_touch_sens_preset(uint8_t preset, bool notify_webusb);
void config_set_mouse_sens_preset(uint8_t preset, bool notify_webusb);
void config_set_deadzone_preset(uint8_t preset, bool notify_webusb);

uint8_t config_get_touch_sens_value(uint8_t index);
double config_get_mouse_sens_value(uint8_t index);
float config_get_deadzone_value(uint8_t index);

void config_set_touch_sens_values(uint8_t* values);
void config_set_mouse_sens_values(double* values);
void config_set_deadzone_values(float* values);

void config_set_log_level(LogLevel log_level);
void config_set_log_mask(LogMask log_mask);
void config_set_long_calibration(bool value);
void config_set_swap_gyros(bool value);
void config_set_touch_invert_polarity(bool value);

// Profiles.
uint8_t config_get_profile();
void config_set_profile(uint8_t profile);
CtrlProfile* config_profile_read(uint8_t index);
void config_profile_write(uint8_t index);
void config_profile_set_sync(uint8_t index, bool state);
void config_profile_default_all();
void config_profile_default(uint8_t indexTo, int8_t indexFrom);
void config_profile_default_home(CtrlProfile *profile);
void config_profile_default_fps_fusion(CtrlProfile *profile);
void config_profile_default_fps_wasd(CtrlProfile *profile);
void config_profile_default_racing(CtrlProfile *profile);
void config_profile_default_flight(CtrlProfile *profile);
void config_profile_default_console(CtrlProfile *profile);
void config_profile_default_console_legacy(CtrlProfile *profile);
void config_profile_default_desktop(CtrlProfile *profile);
void config_profile_default_rts(CtrlProfile *profile);
void config_profile_default_custom(CtrlProfile *profile);
void config_profile_overwrite(uint8_t indexTo, int8_t indexFrom);

// Problems.
void config_set_problem_calibration(bool state);
void config_set_problem_gyro(bool state);
void config_ignore_problems();
bool config_problems_are_pending();
