// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include <pico/time.h>
#include "config.h"
#include "profile.h"
#include "bus.h"
#include "pin.h"
#include "hid.h"
#include "led.h"
#include "logging.h"
#include "helper.h"

Profile profiles[16];
uint8_t profile_active_index = -1;
bool profile_led_lock = false;  // Extern.
bool profile_pending_reboot = false;  // Extern.
bool pending_reset = false;
bool home_is_active = false;
bool home_gamepad_is_active = false;
bool enabled_all = true;
bool enabled_abxy = true;
Button home;

void Profile__report(Profile *self) {
    if (!enabled_all) return;
    bus_i2c_io_cache_update();
    home.report(&home);
    self->select_1.report(&self->select_1);
    self->select_2.report(&self->select_2);
    self->start_2.report(&self->start_1);
    self->start_1.report(&self->start_2);
    if (enabled_abxy) {
        self->a.report(&self->a);
        self->b.report(&self->b);
        self->x.report(&self->x);
        self->y.report(&self->y);
    }
    self->dpad_left.report(&self->dpad_left);
    self->dpad_right.report(&self->dpad_right);
    self->dpad_up.report(&self->dpad_up);
    self->dpad_down.report(&self->dpad_down);
    self->l1.report(&self->l1);
    self->r1.report(&self->r1);
    self->l2.report(&self->l2);
    self->r2.report(&self->r2);
    self->l4.report(&self->l4);
    self->r4.report(&self->r4);
    self->thumbstick.report(&self->thumbstick);
    self->dhat.report(&self->dhat);
    self->rotary.report(&self->rotary);
    self->gyro.report(&self->gyro);
}

void Profile__reset(Profile *self) {
    self->select_1.reset(&self->select_1);
    self->select_2.reset(&self->select_2);
    self->start_2.reset(&self->start_1);
    self->start_1.reset(&self->start_2);
    self->a.reset(&self->a);
    self->b.reset(&self->b);
    self->x.reset(&self->x);
    self->y.reset(&self->y);
    self->dpad_left.reset(&self->dpad_left);
    self->dpad_right.reset(&self->dpad_right);
    self->dpad_up.reset(&self->dpad_up);
    self->dpad_down.reset(&self->dpad_down);
    self->l1.reset(&self->l1);
    self->l2.reset(&self->l2);
    self->r1.reset(&self->r1);
    self->r2.reset(&self->r2);
    self->thumbstick.reset(&self->thumbstick);
    self->rotary.reset(&self->rotary);
    self->gyro.reset(&self->gyro);
}

Tuple16 Profile__get_section(Profile *self, ProfileSection section) {
    Button button;
    switch(section) {
        case_if SECTION_A:          button = self->a;
        case_if SECTION_B:          button = self->b;
        case_if SECTION_X:          button = self->x;
        case_if SECTION_Y:          button = self->y;
        case_if SECTION_DPAD_LEFT:  button = self->dpad_left;
        case_if SECTION_DPAD_RIGHT: button = self->dpad_right;
        case_if SECTION_DPAD_UP:    button = self->dpad_up;
        case_if SECTION_DPAD_DOWN:  button = self->dpad_down;
        case_if SECTION_SELECT_1:   button = self->select_1;
        case_if SECTION_SELECT_2:   button = self->select_2;
        case_if SECTION_START_1:    button = self->start_1;
        case_if SECTION_START_2:    button = self->start_2;
        case_if SECTION_L1:         button = self->l1;
        case_if SECTION_L2:         button = self->l2;
        case_if SECTION_L4:         button = self->l4;
        case_if SECTION_R1:         button = self->r1;
        case_if SECTION_R2:         button = self->r2;
        case_if SECTION_R4:         button = self->r4;
    }
    return (Tuple16){
        button.mode,
        button.actions[0],
        button.actions[1],
        button.actions[2],
        button.actions[3],
        button.actions_secondary[0],
        button.actions_secondary[1],
        button.actions_secondary[2],
        button.actions_secondary[3],
    };
}

Profile Profile_ () {
    Profile profile;
    profile.report = Profile__report;
    profile.reset = Profile__reset;
    profile.get_section = Profile__get_section;
    return profile;
}

void profile_reset_all() {
    config_tune_set_mode(0);
    for(uint8_t i=0; i<=8; i++) {
        profiles[i].reset(&profiles[i]);
    }
}

void profile_update_leds() {
    if (profile_led_lock) return;
    if (home_is_active) {
        led_shape_all_on();
        if (profile_active_index == 1) led_blink_mask(LED_MASK_UP);
        if (profile_active_index == 2) led_blink_mask(LED_MASK_RIGHT);
        if (profile_active_index == 3) led_blink_mask(LED_MASK_DOWN);
        if (profile_active_index == 4) led_blink_mask(LED_MASK_LEFT);
        if (profile_active_index == 5) led_blink_mask(LED_MASK_TRIANGLE_UP);
        if (profile_active_index == 6) led_blink_mask(LED_MASK_TRIANGLE_RIGHT);
        if (profile_active_index == 7) led_blink_mask(LED_MASK_TRIANGLE_DOWN);
        if (profile_active_index == 8) led_blink_mask(LED_MASK_TRIANGLE_LEFT);
    } else {
        led_shape_all_off();
        if (profile_active_index == 1) led_mask(LED_MASK_UP);
        if (profile_active_index == 2) led_mask(LED_MASK_RIGHT);
        if (profile_active_index == 3) led_mask(LED_MASK_DOWN);
        if (profile_active_index == 4) led_mask(LED_MASK_LEFT);
        if (profile_active_index == 5) led_mask(LED_MASK_TRIANGLE_UP);
        if (profile_active_index == 6) led_mask(LED_MASK_TRIANGLE_RIGHT);
        if (profile_active_index == 7) led_mask(LED_MASK_TRIANGLE_DOWN);
        if (profile_active_index == 8) led_mask(LED_MASK_TRIANGLE_LEFT);
    }
}

void profile_report_active() {
    if (profile_pending_reboot && !home_is_active) config_reboot();
    if (pending_reset) {
        hid_matrix_reset();
        profile_reset_all();
        pending_reset = false;
    }
    Profile* profile = profile_get_active(false);
    profile->report(profile);
}

void profile_set_home(bool state) {
    info("Profile: Home %s\n", state ? "on" : "off");
    home_is_active = state;
    if (state) led_shape_all_on();
    else profile_update_leds();
    pending_reset = true;
}

void profile_set_home_gamepad(bool state) {
    home_gamepad_is_active = state;
    if (state) led_shape_all_off();
    else profile_update_leds();
}

void profile_set_active(uint8_t index) {
    if (index != profile_active_index) {
        info("Profile: Profile %i\n", index);
        profile_active_index = index;
        config_set_profile(index);
    }
    profile_update_leds();
}

Profile* profile_get_active(bool strict) {
    if (strict) {
        return &profiles[profile_active_index];
    } else {
        if (home_is_active) return &profiles[PROFILE_HOME];
        else if (home_gamepad_is_active) return &profiles[PROFILE_CONSOLE_LEGACY];
        else return &profiles[profile_active_index];
    }
}

Profile* profile_get_profiles() {
    return profiles;
}

void profile_enable_all(bool value) {
    enabled_all = value;
}

void profile_enable_abxy(bool value) {
    enabled_abxy = value;
}

void profile_init() {
    info("INIT: Profiles\n");
    home = Button_(
        PIN_HOME,
        HOLD_DOUBLE_PRESS,
        ACTIONS(PROC_HOME),
        ACTIONS(GAMEPAD_HOME, PROC_HOME_GAMEPAD)
    );
    profiles[PROFILE_HOME] =           profile_init_home();
    profiles[PROFILE_FPS_FUSION] =     profile_init_fps_fusion();
    profiles[PROFILE_FPS_WASD] =       profile_init_fps_wasd();
    profiles[PROFILE_CONSOLE] =        profile_init_console();
    profiles[PROFILE_CONSOLE_LEGACY] = profile_init_console_legacy();
    profiles[PROFILE_DESKTOP] =        profile_init_desktop();
    profiles[PROFILE_RACING] =         profile_init_racing();
    profiles[PROFILE_FLIGHT] =         profile_init_none();  // TODO: Flight
    profiles[PROFILE_RTS] =            profile_init_none();  // TODO: RTS.
    profile_set_active(config_get_profile());
}
