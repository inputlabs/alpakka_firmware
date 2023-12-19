// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include <string.h>
#include <pico/time.h>
#include <hardware/gpio.h>
#include "config.h"
#include "profile.h"
#include "hid.h"
#include "bus.h"
#include "pin.h"
#include "common.h"

bool Button__is_pressed(Button *self) {
    if (self->pin == PIN_NONE) return false;
    // Virtual buttons.
    else if (self->pin == PIN_VIRTUAL) {
        if (self->virtual_press) {
            self->virtual_press = false;
            return true;
        } else {
            return false;
        }
    }
    // Buttons connected directly to Pico.
    else if (is_between(self->pin, PIN_GROUP_PICO, PIN_GROUP_PICO_END)) {
        return !gpio_get(self->pin);
    }
    // Buttons connected to 1st IO expander.
    else if (is_between(self->pin, PIN_GROUP_IO_0, PIN_GROUP_IO_0_END)) {
        return bus_i2c_io_cache_read(0, self->pin - PIN_GROUP_IO_0);
    }
    // Buttons connected to 2nd IO expander.
    else if (is_between(self->pin, PIN_GROUP_IO_1, PIN_GROUP_IO_1_END)) {
        return bus_i2c_io_cache_read(1, self->pin - PIN_GROUP_IO_1);
    }
}

void Button__report(Button *self) {
    if (self->mode == NORMAL) self->handle_normal(self);
    else if (self->mode == STICKY) self->handle_sticky(self);
    else if (self->mode == HOLD_DOUBLE_PRESS) self->handle_hold_double_press(self);
    else if (self->mode == HOLD_EXCLUSIVE) {
        self->handle_hold_exclusive(self, CFG_HOLD_EXCLUSIVE_TIME);
    }
    else if (self->mode == HOLD_EXCLUSIVE_LONG) {
        self->handle_hold_exclusive(self, CFG_HOLD_EXCLUSIVE_LONG_TIME);
    }
    else if (self->mode == HOLD_OVERLAP) {
        self->handle_hold_overlap(self, CFG_HOLD_OVERLAP_TIME);
    }
    else if (self->mode == HOLD_OVERLAP_LONG) {
        self->handle_hold_overlap(self, CFG_HOLD_OVERLAP_LONG_TIME);
    }
}

void Button__handle_normal(Button *self) {
    if (time_us_64() < self->press_timestamp + CFG_PRESS_DEBOUNCE * 1000) {
        return;
    };
    bool pressed = self->is_pressed(self);
    if(pressed && !self->state) {
        hid_press_multiple(self->actions);
        self->state = true;
        self->press_timestamp = time_us_64();
        return;
    }
    if((!pressed) && self->state) {
        hid_release_multiple(self->actions);
        self->state = false;
        self->press_timestamp = time_us_64();;
        return;
    }
}

void Button__handle_sticky(Button *self) {
    bool pressed = self->is_pressed(self);
    if(pressed && !self->state) {
        self->state = true;
        hid_press_multiple(self->actions);
        hid_press_multiple(self->actions_secondary);
        return;
    }
    if((!pressed) && self->state) {
        self->state = false;
        hid_release_multiple(self->actions_secondary);
        return;
    }
}

void Button__handle_hold_exclusive(Button *self, uint16_t time) {
    bool pressed = self->is_pressed(self);
    if(pressed && !self->state && !self->state_secondary) {
        self->state = true;
        self->hold_timestamp = time_us_64();
        return;
    }
    if(pressed && self->state && !self->state_secondary) {
        uint64_t hold_time_us = time * 1000;
        if (time_us_64() > self->hold_timestamp + hold_time_us) {
            hid_press_multiple(self->actions_secondary);
            self->state = false;
            self->state_secondary = true;
        }
    }
    if(!pressed) {
        if (self->state) {
            hid_press_multiple(self->actions);
            hid_release_multiple_later(self->actions, 100);
            self->state = false;
            return;
        }
        if (self->state_secondary) {
            hid_release_multiple(self->actions_secondary);
            self->state_secondary = false;
            return;
        }
    }
}

void Button__handle_hold_overlap(Button *self, uint16_t time) {
    bool pressed = self->is_pressed(self);
    if(pressed && !self->state && !self->state_secondary) {
        hid_press_multiple(self->actions);
        self->state = true;
        self->hold_timestamp = time_us_64();
        return;
    }
    if(pressed && self->state && !self->state_secondary) {
        uint64_t hold_time_us = time * 1000;
        if (time_us_64() > self->hold_timestamp + hold_time_us) {
            hid_press_multiple(self->actions_secondary);
            self->state_secondary = true;
        }
        return;
    }
    if(!pressed) {
        if (self->state) {
            hid_release_multiple(self->actions);
            self->state = false;
        }
        if (self->state_secondary) {
            hid_release_multiple(self->actions_secondary);
            self->state_secondary = false;
        }
        return;
    }
}

void Button__handle_hold_double_press(Button *self) {
    bool pressed = self->is_pressed(self);
    if(pressed && !self->state && !self->state_secondary) {
        uint64_t now = time_us_64();
        uint64_t threshold = self->press_timestamp + (CFG_DOUBLE_PRESS * 1000);
        if (now > threshold) {
            // Simple press.
            hid_press_multiple(self->actions);
            self->state = true;
            self->press_timestamp = now;
        } else {
            // Double press.
            hid_press_multiple(self->actions_secondary);
            self->state_secondary = true;
        }
        return;
    }
    if((!pressed) && self->state) {
        // Release after simple press.
        hid_release_multiple(self->actions);
        self->state = false;
        return;
    }
    if((!pressed) && self->state_secondary) {
        // Release after double press.
        hid_release_multiple(self->actions_secondary);
        self->state_secondary = false;
        return;
    }
}

void Button__reset(Button *self) {
    self->state = false;
}

// Init.
Button Button_ (
    uint8_t pin,
    ButtonMode mode,
    Actions actions,
    Actions actions_secondary
) {
    if (pin) {
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_IN);
        gpio_pull_up(pin);
    }
    Button button;
    button.is_pressed = Button__is_pressed;
    button.report = Button__report;
    button.reset = Button__reset;
    button.handle_normal = Button__handle_normal;
    button.handle_sticky = Button__handle_sticky;
    button.handle_hold_exclusive = Button__handle_hold_exclusive;
    button.handle_hold_overlap = Button__handle_hold_overlap;
    button.handle_hold_double_press = Button__handle_hold_double_press;
    button.pin = pin;
    button.mode = mode;
    button.state = false;
    button.virtual_press = false;
    button.state_secondary = false;
    button.press_timestamp = 0;
    button.hold_timestamp = 0;
    memcpy(button.actions, actions, 4);
    memcpy(button.actions_secondary, actions_secondary, 4);
    return button;
}

// Alternative init.
Button Button_from_ctrl(uint8_t pin, CtrlSection section) {
    return Button_(
        pin,
        section.button.mode,
        section.button.actions,
        section.button.actions_secondary
    );
}
