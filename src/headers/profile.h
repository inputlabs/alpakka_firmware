// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#pragma once
#include "button.h"
#include "thumbstick.h"
#include "dhat.h"
#include "rotary.h"
#include "gyro.h"
#include "webusb.h"
#include "common.h"

#define PROFILE_SLOTS 14

typedef enum ProfileIndex_enum {
    PROFILE_HOME,
    PROFILE_FPS_FUSION,
    PROFILE_RACING,
    PROFILE_CONSOLE,
    PROFILE_DESKTOP,
    PROFILE_FPS_WASD,
    PROFILE_FLIGHT,
    PROFILE_CONSOLE_LEGACY,
    PROFILE_RTS,
    PROFILE_CUSTOM_1,
    PROFILE_CUSTOM_2,
    PROFILE_CUSTOM_3,
    PROFILE_CUSTOM_4,
    PROFILE_HOME_GAMEPAD,
} ProfileIndex;

typedef struct Profile_struct Profile;
struct Profile_struct {
    void (*report) (Profile *self);
    void (*reset) (Profile *self);
    void (*load_from_config) (Profile *self, CtrlProfile *profile);
    Button select_1;
    Button select_2;
    Button start_1;
    Button start_2;
    Button dpad_down;
    Button dpad_right;
    Button dpad_left;
    Button dpad_up;
    Button a;
    Button b;
    Button x;
    Button y;
    Button hat_down;
    Button hat_right;
    Button hat_left;
    Button hat_up;
    Button hat_in;
    Button l1;
    Button l2;
    Button l3;
    Button l4;
    Button r1;
    Button r2;
    Button r4;
    Thumbstick left_thumbstick;
    Thumbstick right_thumbstick;
    Dhat dhat;
    Rotary rotary;
    Gyro gyro;
};
Profile Profile_ ();

void profile_init();
void profile_report_active();
void profile_set_home(bool state);
void profile_set_home_gamepad(bool state);
void profile_set_active(uint8_t index);
void profile_set_lock_leds(bool lock);
void profile_update_leds();
void profile_enable_all(bool value);
void profile_enable_abxy(bool value);
Profile* profile_get(uint8_t index);
Profile* profile_get_active(bool strict);
uint8_t profile_get_active_index(bool strict);

extern bool profile_led_lock;
extern bool profile_pending_reboot;
