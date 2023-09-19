// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#pragma once
#include <stdint.h>
#include <math.h>
#include <stdbool.h>

#define NVM_CONFIG_ADDR 0x001D0000
#define NVM_CONFIG_HEADER 0b01010101
#define NVM_STRUCT_VERSION 11

#define PROTOCOL_XINPUT_WIN 0
#define PROTOCOL_XINPUT_UNIX 1
#define PROTOCOL_GENERIC 2

#define CFG_LED_BRIGHTNESS 0.2

#define CFG_TICK_FREQUENCY 250  // Hz.
#define CFG_TICK_INTERVAL  (1000 / CFG_TICK_FREQUENCY)
#define CFG_HID_REPORT_PRIORITY_RATIO 8

#define CFG_CALIBRATION_SAMPLES_GYRO 200000  // How long the calibration takes.
#define CFG_CALIBRATION_SAMPLES_ACCEL 100000  // How long the calibration takes.
#define CFG_IMU_TICK_SAMPLES 128  // Multi-sampling per pooling cycle.

#define CFG_GYRO_SENSITIVITY  pow(2, -9) * 1.45
#define CFG_GYRO_SENSITIVITY_X  CFG_GYRO_SENSITIVITY * 1
#define CFG_GYRO_SENSITIVITY_Y  CFG_GYRO_SENSITIVITY * 1
#define CFG_GYRO_SENSITIVITY_Z  CFG_GYRO_SENSITIVITY * 1
#define CFG_GYRO_SENSITIVITY_MULTIPLIER_LOW 1.0
#define CFG_GYRO_SENSITIVITY_MULTIPLIER_MID 4.0 / 3.0
#define CFG_GYRO_SENSITIVITY_MULTIPLIER_HIGH 2.0
#define CFG_MOUSE_WHEEL_DEBOUNCE 1000
#define CFG_ACCEL_CORRECTION_SMOOTH 50  // Number of averaged samples for the correction vector.
#define CFG_ACCEL_CORRECTION_RATE 0.0007  // How fast the correction is applied.

#define CFG_PRESS_DEBOUNCE 50  // Milliseconds.
#define CFG_HOLD_EXCLUSIVE_TIME 200  // Milliseconds.
#define CFG_HOLD_EXCLUSIVE_LONG_TIME 2000  // Milliseconds.
#define CFG_HOLD_OVERLAP_TIME 250  // Milliseconds.
#define CFG_HOLD_OVERLAP_LONG_TIME 2000  // Milliseconds.
#define CFG_DOUBLE_PRESS 300  // Milliseconds.

#define CFG_THUMBSTICK_DEADZONE_LOW 0.07
#define CFG_THUMBSTICK_DEADZONE_MID 0.10
#define CFG_THUMBSTICK_DEADZONE_HIGH 0.15
#define CFG_THUMBSTICK_SATURATION 1.8
#define CFG_THUMBSTICK_INNER_RADIUS 0.75

#define CFG_DHAT_DEBOUNCE_TIME 100  // Milliseconds.

typedef struct {
    uint8_t header;
    uint8_t config_version;
    int8_t protocol;
    int8_t sens_mouse;
    int8_t sens_touch;
    int8_t deadzone;
    int8_t vibration;
    uint8_t profile;
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
    uint8_t padding[256];
} config_nvm_t;

void config_init();
void config_read(config_nvm_t* config);
void config_set_profile(uint8_t profile);
uint8_t config_get_profile();
void config_set_thumbstick_offset(float x, float y);
void config_set_gyro_offset(double ax, double ay, double az, double bx, double by, double bz);
void config_set_accel_offset(double ax, double ay, double az, double bx, double by, double bz);
uint8_t config_get_protocol();
void config_tune_set_mode(uint8_t mode);
void config_tune(bool direction);
void config_calibrate();
void config_reboot();
void config_bootsel();
void config_factory();
void config_write_init();
void config_print();
void config_set_pcb_gen(uint8_t gen);
uint8_t config_get_pcb_gen();

uint8_t config_get_protocol();
uint8_t config_get_touch_sens();
uint8_t config_get_mouse_sens();
uint8_t config_get_deadzone();
void config_set_protocol(uint8_t preset);
void config_set_touch_sens(uint8_t preset, bool notify_webusb);
void config_set_mouse_sens(uint8_t preset, bool notify_webusb);
void config_set_deadzone(uint8_t preset, bool notify_webusb);
