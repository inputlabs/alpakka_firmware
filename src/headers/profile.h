// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#pragma once
#include "button.h"
#include "thumbstick.h"
#include "dhat.h"
#include "rotary.h"
#include "gyro.h"
#include "helper.h"

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
} ProfileIndex;

typedef enum ProfileSection_enum {
    SECTION_NAME = 1,
    SECTION_A = 10,
    SECTION_B,
    SECTION_X,
    SECTION_Y,
    SECTION_DPAD_LEFT,
    SECTION_DPAD_RIGHT,
    SECTION_DPAD_UP,
    SECTION_DPAD_DOWN,
    SECTION_SELECT_1,
    SECTION_SELECT_2,
    SECTION_START_1,
    SECTION_START_2,
    SECTION_L1,
    SECTION_L2,
    SECTION_L4,
    SECTION_R1,
    SECTION_R2,
    SECTION_R4,
} ProfileSection;

typedef struct Profile_struct Profile;
struct Profile_struct {
    void (*report) (Profile *self);
    void (*reset) (Profile *self);
    Tuple16 (*get_section) (Profile *self, ProfileSection section);
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
    Thumbstick thumbstick;
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
Profile* profile_get_active(bool strict);
Profile* profile_get_profiles();

Profile profile_init_none();
Profile profile_init_home();
Profile profile_init_fps_fusion();
Profile profile_init_fps_wasd();
Profile profile_init_console();
Profile profile_init_console_legacy();
Profile profile_init_desktop();
Profile profile_init_racing();

extern bool profile_led_lock;
extern bool profile_pending_reboot;
