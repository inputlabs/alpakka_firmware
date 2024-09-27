// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include <pico/time.h>
#include <string.h>
#include "config.h"
#include "profile.h"
#include "button.h"
#include "bus.h"
#include "glyph.h"
#include "pin.h"
#include "hid.h"
#include "led.h"
#include "webusb.h"
#include "logging.h"
#include "common.h"

Profile profiles[PROFILE_SLOTS];
uint8_t profile_active_index = -1;
bool profile_led_lock = false;  // Extern.
bool profile_pending_reboot = false;  // Extern.
bool pending_reset = false;
uint8_t pending_reset_keep;  // Action that must be kept between resets.
bool home_is_active = false;
bool home_gamepad_is_active = false;
bool enabled_all = true;
bool enabled_abxy = true;
Button home;

void Profile__report(Profile *self) {
    if (!enabled_all) return;
    bus_i2c_io_cache_update();
    home.report(&home);
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
    self->select_1.report(&self->select_1);
    self->select_2.report(&self->select_2);
    self->start_2.report(&self->start_1);
    self->start_1.report(&self->start_2);
    self->l1.report(&self->l1);
    self->r1.report(&self->r1);
    self->l2.report(&self->l2);
    self->r2.report(&self->r2);
    self->l4.report(&self->l4);
    self->r4.report(&self->r4);
    self->dhat.report(&self->dhat);
    self->rotary.report(&self->rotary);
    self->thumbstick.report(&self->thumbstick);
    self->gyro.report(&self->gyro);
}

void Profile__reset(Profile *self) {
    self->a.reset(&self->a);
    self->b.reset(&self->b);
    self->x.reset(&self->x);
    self->y.reset(&self->y);
    self->dpad_left.reset(&self->dpad_left);
    self->dpad_right.reset(&self->dpad_right);
    self->dpad_up.reset(&self->dpad_up);
    self->dpad_down.reset(&self->dpad_down);
    self->select_1.reset(&self->select_1);
    self->select_2.reset(&self->select_2);
    self->start_2.reset(&self->start_1);
    self->start_1.reset(&self->start_2);
    self->l1.reset(&self->l1);
    self->l2.reset(&self->l2);
    self->r1.reset(&self->r1);
    self->r2.reset(&self->r2);
    self->l4.reset(&self->l4);
    self->r4.reset(&self->r4);
    self->dhat.reset(&self->dhat);
    self->rotary.reset(&self->rotary);
    self->thumbstick.reset(&self->thumbstick);
    self->gyro.reset(&self->gyro);
}

void Profile__load_from_config(Profile *self, CtrlProfile *profile) {
    // Buttons.
    self->a =          Button_from_ctrl(PIN_A,          profile->sections[SECTION_A]);
    self->b =          Button_from_ctrl(PIN_B,          profile->sections[SECTION_B]);
    self->x =          Button_from_ctrl(PIN_X,          profile->sections[SECTION_X]);
    self->y =          Button_from_ctrl(PIN_Y,          profile->sections[SECTION_Y]);
    self->dpad_left =  Button_from_ctrl(PIN_DPAD_LEFT,  profile->sections[SECTION_DPAD_LEFT]);
    self->dpad_right = Button_from_ctrl(PIN_DPAD_RIGHT, profile->sections[SECTION_DPAD_RIGHT]);
    self->dpad_up =    Button_from_ctrl(PIN_DPAD_UP,    profile->sections[SECTION_DPAD_UP]);
    self->dpad_down =  Button_from_ctrl(PIN_DPAD_DOWN,  profile->sections[SECTION_DPAD_DOWN]);
    self->select_1 =   Button_from_ctrl(PIN_SELECT_1,   profile->sections[SECTION_SELECT_1]);
    self->select_2 =   Button_from_ctrl(PIN_SELECT_2,   profile->sections[SECTION_SELECT_2]);
    self->start_2 =    Button_from_ctrl(PIN_START_2,    profile->sections[SECTION_START_2]);
    self->start_1 =    Button_from_ctrl(PIN_START_1,    profile->sections[SECTION_START_1]);
    self->l1 =         Button_from_ctrl(PIN_L1,         profile->sections[SECTION_L1]);
    self->l2 =         Button_from_ctrl(PIN_L2,         profile->sections[SECTION_L2]);
    self->r1 =         Button_from_ctrl(PIN_R1,         profile->sections[SECTION_R1]);
    self->r2 =         Button_from_ctrl(PIN_R2,         profile->sections[SECTION_R2]);
    self->l4 =         Button_from_ctrl(PIN_L4,         profile->sections[SECTION_L4]);
    self->r4 =         Button_from_ctrl(PIN_R4,         profile->sections[SECTION_R4]);
    // Dhat.
    self->dhat = Dhat_(
        Button_from_ctrl(PIN_VIRTUAL, profile->sections[SECTION_DHAT_LEFT]),
        Button_from_ctrl(PIN_VIRTUAL, profile->sections[SECTION_DHAT_RIGHT]),
        Button_from_ctrl(PIN_VIRTUAL, profile->sections[SECTION_DHAT_UP]),
        Button_from_ctrl(PIN_VIRTUAL, profile->sections[SECTION_DHAT_DOWN]),
        Button_from_ctrl(PIN_VIRTUAL, profile->sections[SECTION_DHAT_UL]),
        Button_from_ctrl(PIN_VIRTUAL, profile->sections[SECTION_DHAT_UR]),
        Button_from_ctrl(PIN_VIRTUAL, profile->sections[SECTION_DHAT_DL]),
        Button_from_ctrl(PIN_VIRTUAL, profile->sections[SECTION_DHAT_DR]),
        Button_from_ctrl(PIN_VIRTUAL, profile->sections[SECTION_DHAT_PUSH])
    );
    // Rotary.
    CtrlRotary up = profile->sections[SECTION_ROTARY_UP].rotary;
    CtrlRotary down = profile->sections[SECTION_ROTARY_DOWN].rotary;
    Rotary rotary = Rotary_();
    rotary.config_mode(&rotary, 0, up.actions_0, down.actions_0);
    rotary.config_mode(&rotary, 1, up.actions_1, down.actions_1);
    rotary.config_mode(&rotary, 2, up.actions_2, down.actions_2);
    rotary.config_mode(&rotary, 3, up.actions_3, down.actions_3);
    rotary.config_mode(&rotary, 4, up.actions_4, down.actions_4);
    self->rotary = rotary;
    // Thumbstick.
    CtrlThumbstick ctrl_thumbtick = profile->sections[SECTION_THUMBSTICK].thumbstick;
    self->thumbstick = Thumbstick_(
        ctrl_thumbtick.mode,
        ctrl_thumbtick.distance_mode,
        ctrl_thumbtick.deadzone_override,
        ctrl_thumbtick.deadzone / 100.0,
        ctrl_thumbtick.antideadzone / 100.0,
        (int8_t)ctrl_thumbtick.overlap / 100.0
    );
    if (ctrl_thumbtick.mode == THUMBSTICK_MODE_4DIR) {
        self->thumbstick.config_4dir(
            &(self->thumbstick),
            Button_from_ctrl(PIN_VIRTUAL, profile->sections[SECTION_THUMBSTICK_LEFT]),
            Button_from_ctrl(PIN_VIRTUAL, profile->sections[SECTION_THUMBSTICK_RIGHT]),
            Button_from_ctrl(PIN_VIRTUAL, profile->sections[SECTION_THUMBSTICK_UP]),
            Button_from_ctrl(PIN_VIRTUAL, profile->sections[SECTION_THUMBSTICK_DOWN]),
            Button_from_ctrl(PIN_L3,      profile->sections[SECTION_THUMBSTICK_PUSH]),
            Button_from_ctrl(PIN_VIRTUAL, profile->sections[SECTION_THUMBSTICK_INNER]),
            Button_from_ctrl(PIN_VIRTUAL, profile->sections[SECTION_THUMBSTICK_OUTER])
        );
    }
    if (ctrl_thumbtick.mode == THUMBSTICK_MODE_ALPHANUMERIC) {
        // Iterate sections.
        for(uint8_t s=0; s<4; s++) {
            // Iterate groups.
            for(uint8_t g=0; g<11; g++) {
                CtrlGlyph ctrl_glyph = profile->sections[SECTION_GLYPHS_0+s].glyphs.glyphs[g];
                Glyph glyph = {0};
                glyph_decode(glyph, ctrl_glyph.glyph);
                self->thumbstick.config_glyphstick(
                    &(self->thumbstick),
                    ctrl_glyph.actions,
                    glyph
                );
            }
        }
        uint8_t dir = 0;
        // Iterate sections.
        for(uint8_t s=0; s<4; s++) {
            // Iterate groups.
            for(uint8_t g=0; g<2; g++) {
                CtrlDaisyGroup group = profile->sections[SECTION_DAISY_0+s].daisy.groups[g];
                self->thumbstick.config_daisywheel(&(self->thumbstick), dir, 0, group.actions_a);
                self->thumbstick.config_daisywheel(&(self->thumbstick), dir, 1, group.actions_b);
                self->thumbstick.config_daisywheel(&(self->thumbstick), dir, 2, group.actions_x);
                self->thumbstick.config_daisywheel(&(self->thumbstick), dir, 3, group.actions_y);
                dir += 1;
            }
        }
    }
    // Gyro.
    CtrlGyro ctrl_gyro = profile->sections[SECTION_GYRO].gyro;
    CtrlGyroAxis ctrl_gyro_x = profile->sections[SECTION_GYRO_X].gyro_axis;
    CtrlGyroAxis ctrl_gyro_y = profile->sections[SECTION_GYRO_Y].gyro_axis;
    CtrlGyroAxis ctrl_gyro_z = profile->sections[SECTION_GYRO_Z].gyro_axis;
    self->gyro = Gyro_(
        ctrl_gyro.mode,
        ctrl_gyro.engage
    );
    self->gyro.config_x(
        &(self->gyro),
        (int8_t)ctrl_gyro_x.angle_min,
        (int8_t)ctrl_gyro_x.angle_max,
        ctrl_gyro_x.actions_neg,
        ctrl_gyro_x.actions_pos
    );
    self->gyro.config_y(
        &(self->gyro),
        (int8_t)ctrl_gyro_y.angle_min,
        (int8_t)ctrl_gyro_y.angle_max,
        ctrl_gyro_y.actions_neg,
        ctrl_gyro_y.actions_pos
    );
    self->gyro.config_z(
        &(self->gyro),
        (int8_t)ctrl_gyro_z.angle_min,
        (int8_t)ctrl_gyro_z.angle_max,
        ctrl_gyro_z.actions_neg,
        ctrl_gyro_z.actions_pos
    );
}

Profile Profile_ () {
    Profile profile;
    profile.report = Profile__report;
    profile.reset = Profile__reset;
    profile.load_from_config = Profile__load_from_config;
    return profile;
}

// ============================================================================
// Independent functions.

void profile_reset_all_profiles() {
    config_tune_set_mode(0);
    for(uint8_t i=0; i<PROFILE_SLOTS; i++) {
        profiles[i].reset(&profiles[i]);
    }
}

void profile_update_leds() {
    if (profile_led_lock) return;
    if (home_is_active) {
        led_static_mask(LED_ALL);
        if (profile_active_index == 1) led_blink_mask(LED_UP);
        if (profile_active_index == 2) led_blink_mask(LED_RIGHT);
        if (profile_active_index == 3) led_blink_mask(LED_DOWN);
        if (profile_active_index == 4) led_blink_mask(LED_LEFT);
        if (profile_active_index == 5) led_blink_mask(LED_TRIANGLE_UP);
        if (profile_active_index == 6) led_blink_mask(LED_TRIANGLE_RIGHT);
        if (profile_active_index == 7) led_blink_mask(LED_TRIANGLE_DOWN);
        if (profile_active_index == 8) led_blink_mask(LED_TRIANGLE_LEFT);
        if (profile_active_index == 9) led_blink_mask(LED_UP+LED_RIGHT);
        if (profile_active_index == 10) led_blink_mask(LED_RIGHT+LED_DOWN);
        if (profile_active_index == 11) led_blink_mask(LED_DOWN+LED_LEFT);
        if (profile_active_index == 12) led_blink_mask(LED_LEFT+LED_UP);
        led_set_mode(LED_MODE_BLINK);
    } else {
        if (profile_active_index == 1) led_idle_mask(LED_UP);
        if (profile_active_index == 2) led_idle_mask(LED_RIGHT);
        if (profile_active_index == 3) led_idle_mask(LED_DOWN);
        if (profile_active_index == 4) led_idle_mask(LED_LEFT);
        if (profile_active_index == 5) led_idle_mask(LED_TRIANGLE_UP);
        if (profile_active_index == 6) led_idle_mask(LED_TRIANGLE_RIGHT);
        if (profile_active_index == 7) led_idle_mask(LED_TRIANGLE_DOWN);
        if (profile_active_index == 8) led_idle_mask(LED_TRIANGLE_LEFT);
        if (profile_active_index == 9) led_idle_mask(LED_UP+LED_RIGHT);
        if (profile_active_index == 10) led_idle_mask(LED_RIGHT+LED_DOWN);
        if (profile_active_index == 11) led_idle_mask(LED_DOWN+LED_LEFT);
        if (profile_active_index == 12) led_idle_mask(LED_LEFT+LED_UP);
        led_set_mode(LED_MODE_IDLE);
    }
}

void profile_reset_all() {
    // Reset HID state matrix. Optionally keep certain actions.
    hid_matrix_reset(pending_reset_keep);
    // Reset all profiles runtimes.
    profile_reset_all_profiles();
    // Reset flags.
    pending_reset = false;
    pending_reset_keep = 0;
}

void profile_report_active() {
    if (profile_pending_reboot && !home_is_active) config_reboot();
    if (pending_reset) profile_reset_all();
    Profile* profile = profile_get_active(false);
    profile->report(profile);
}

void profile_set_home(bool state) {
    info("Profile: Home %s\n", state ? "on" : "off");
    home_is_active = state;
    if (state) {
        led_static_mask(LED_ALL);
        led_set_mode(LED_MODE_ENGAGE);
    } else {
        profile_update_leds();
    }
    pending_reset = true;
}

void profile_set_home_gamepad(bool state) {
    home_gamepad_is_active = state;
    if (state) {
        led_static_mask(LED_NONE);
        led_set_mode(LED_MODE_ENGAGE);
    } else {
        profile_update_leds();
    }
    pending_reset = true;
    pending_reset_keep = GAMEPAD_HOME;  // Do not reset held gamepad home.
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
        else if (home_gamepad_is_active) return &profiles[PROFILE_HOME_GAMEPAD];
        else return &profiles[profile_active_index];
    }
}

Profile* profile_get(uint8_t index) {
    return &profiles[index];
}

uint8_t profile_get_active_index(bool strict) {
    if (strict) {
        return profile_active_index;
    } else {
        if (home_is_active) return 0;
        else return profile_active_index;
    }
}

void profile_enable_all(bool value) {
    enabled_all = value;
}

void profile_enable_abxy(bool value) {
    enabled_abxy = value;
}

void profile_init() {
    info("INIT: Profiles\n");
    // Home button setup.
    Actions actions = {PROC_HOME};
    Actions actions_secondary = {0, 0, 0, 0};
    Actions actions_terciary = {GAMEPAD_HOME, PROC_HOME_GAMEPAD, PROC_IGNORE_LED_WARNINGS};
    home = Button_(PIN_HOME, DOUBLE|IMMEDIATE, actions, actions_secondary, actions_terciary);
    // Profiles setup.
    for(uint8_t i=0; i<PROFILE_SLOTS; i++) {
        profiles[i] = Profile_();
        profiles[i].load_from_config(&(profiles[i]), config_profile_read(i));
    }
    profile_set_active(config_get_profile());
}
