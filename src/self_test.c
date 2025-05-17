// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include "bus.h"
#include "hid.h"
#include "pin.h"
#include "profile.h"
#include "uart.h"
#include "logging.h"
#include "common.h"

void self_test_button_press(const char *buttonName, Button* button) {
    info("Press button '%s': WAITING", buttonName);
    while (!button->is_pressed(button)) {
        uart_listen_serial_limited();
        bus_i2c_io_cache_update();
        sleep_ms(1);
    }
    info("\rPress button '%s': OK     \n", buttonName);
}

void self_test_buttons(Profile* profile) {
    Actions none = {0,};
    Button home = Button_(PIN_HOME, NORMAL, none, none, none);
    self_test_button_press("A", &profile->a);
    self_test_button_press("B", &profile->b);
    self_test_button_press("X", &profile->x);
    self_test_button_press("Y", &profile->y);
    self_test_button_press("D-Pad Left", &profile->dpad_left);
    self_test_button_press("D-Pad Right", &profile->dpad_right);
    self_test_button_press("D-Pad Up", &profile->dpad_up);
    self_test_button_press("D-Pad Down", &profile->dpad_down);
    self_test_button_press("Home", &home);
    self_test_button_press("Select 1", &profile->select_1);
    self_test_button_press("Start 1", &profile->start_1);
    self_test_button_press("Select 2", &profile->select_2);
    self_test_button_press("Start 2", &profile->start_2);
    self_test_button_press("L1", &profile->l1);
    self_test_button_press("R1", &profile->r1);
    self_test_button_press("L2", &profile->l2);
    self_test_button_press("R2", &profile->r2);
    self_test_button_press("L4", &profile->l4);
    self_test_button_press("R4", &profile->r4);
    home.reset(&home);
    profile->reset(profile);
}

void self_test_thumbstick_direction(const char *buttonName, Button* button, Thumbstick* thumbstick) {
    info("Move thumbstick %s: WAITING", buttonName);
    while (!button->virtual_press) {
        uart_listen_serial_limited();
        thumbstick->report(thumbstick);
        sleep_ms(1);
    }
    info("\rMove thumbstick %s: OK     \n", buttonName);
}

void self_test_thumbstick(Thumbstick* thumbstick) {
    self_test_button_press("Thumbstick Push", &thumbstick->push);
    self_test_thumbstick_direction("left", &thumbstick->left, thumbstick);
    self_test_thumbstick_direction("right", &thumbstick->right, thumbstick);
    self_test_thumbstick_direction("up", &thumbstick->up, thumbstick);
    self_test_thumbstick_direction("down", &thumbstick->down, thumbstick);
    thumbstick->reset(thumbstick);
}

void self_test_dhat_press(Dhat* dhat, const char *buttonName, Button* button) {
    info("Press DHat '%s': WAITING", buttonName);
    while (!button->is_pressed(button)) {
        uart_listen_serial_limited();
        bus_i2c_io_cache_update();
        dhat->update(dhat);
        sleep_ms(1);
    }
    info("\rPress DHat '%s': OK     \n", buttonName);
}

void self_test_dhat(Dhat* dhat) {
    self_test_dhat_press(dhat, "left", &(dhat->mid_left));
    self_test_dhat_press(dhat, "right", &(dhat->mid_right));
    self_test_dhat_press(dhat, "up", &(dhat->up_center));
    self_test_dhat_press(dhat, "down", &(dhat->down_center));
    self_test_dhat_press(dhat, "push", &(dhat->mid_center));
}

void self_test_rotary_direction(Rotary* rotary, const char *name, int8_t direction) {
    rotary->increment = 0;
    info("Scroll %s: WAITING", name);
    while (rotary->increment != direction) {
        uart_listen_serial_limited();
        sleep_ms(1);
    }
    info("\rScroll %s: OK     \n", name);
}

void self_test_rotary(Rotary* rotary) {
    self_test_rotary_direction(rotary, "up", 1);
    self_test_rotary_direction(rotary, "down", -1);
    rotary->reset(rotary);
}

void self_test() {
    profile_enable_all(false);
    info("Tests start\n");
    info("===========\n");
    Profile* profile = profile_get_active(true);
    self_test_buttons(profile);
    self_test_thumbstick(&(profile->left_thumbstick));
    #if defined DEVICE_ALPAKKA_V0
        self_test_dhat(&(profile->dhat));
    #elif defined DEVICE_ALPAKKA_V1
        self_test_thumbstick(&(profile->right_thumbstick));
    #endif
    self_test_rotary(&(profile->rotary));
    info("Tests done\n");
    info("==========\n");
    profile->reset(profile);
    profile_enable_all(true);
}
