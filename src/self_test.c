// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include "bus.h"
#include "hid.h"
#include "pin.h"
#include "profile.h"
#include "uart.h"

void self_test_button_press(const char *buttonName, Button* button) {
    printf("Press button '%s': WAITING", buttonName);

    do {
        uart_listen_char_limited();
        sleep_ms(1);
        bus_i2c_io_cache_update();
    } while (!button->is_pressed(button));

    printf("\rPress button '%s': OK     \n", buttonName);
}

void self_test_buttons(Profile* profile) {
    Button home = Button_(PIN_HOME, NORMAL, ACTIONS(KEY_NONE));

    printf("Testing buttons\n");

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

    self_test_button_press("D-Hat Left", &profile->dhat.left);
    self_test_button_press("D-Hat Right", &profile->dhat.right);
    self_test_button_press("D-Hat Up", &profile->dhat.up);
    self_test_button_press("D-Hat Down", &profile->dhat.down);

    self_test_button_press("D-Hat", &profile->dhat.push);

    home.reset(&home);
    profile->reset(profile);
}

void self_test_thumbstick_direction(const char *buttonName, Button* button, Thumbstick* thumbstick) {
    printf("Move thumbstick %s: WAITING", buttonName);

    do {
        uart_listen_char_limited();
        sleep_ms(1);
        thumbstick->report(thumbstick);
    } while (!button->virtual_press);

    printf("\rMove thumbstick %s: OK     \n", buttonName);
}

void self_test_thumbstick(Thumbstick* thumbstick) {
    printf("Testing thumbstick\n");

    self_test_thumbstick_direction("left", &thumbstick->left, thumbstick);
    self_test_thumbstick_direction("right", &thumbstick->right, thumbstick);
    self_test_thumbstick_direction("up", &thumbstick->up, thumbstick);
    self_test_thumbstick_direction("down", &thumbstick->down, thumbstick);

    self_test_button_press("Thumbstick", &thumbstick->push);

    thumbstick->reset(thumbstick);
}

void self_test_rotary(Rotary* rotary) {
    printf("Testing Rotary Wheel\n");

    // TODO test rotary wheel

    rotary->reset(rotary);
}

void self_test() {
    Profile profile = profile_init_none();
    self_test_buttons(&profile);
    self_test_thumbstick(&profile.thumbstick);
    self_test_rotary(&profile.rotary);
    profile.reset(&profile);
    printf("Tests done\n");
}
