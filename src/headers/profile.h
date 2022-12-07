// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#pragma once
#include "button.h"
#include "thumbstick.h"
#include "dhat.h"
#include "rotary.h"
#include "gyro.h"

typedef struct Profile_struct Profile;

struct Profile_struct {
    void (*report) (Profile *self);
    void (*reset) (Profile *self);
    bool (*was_used) (Profile *self);
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
void profile_set_active(uint8_t index);
void profile_set_lock_leds(bool lock);
void profile_update_leds();
Profile profile_init_none();
Profile profile_init_home();
Profile profile_init_fps_fusion();
Profile profile_init_fps_wasd();
Profile profile_init_console();
Profile profile_init_console_legacy();
Profile profile_init_desktop();
extern bool profile_led_lock;
