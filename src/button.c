// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include <string.h>
#include <pico/time.h>
#include <hardware/gpio.h>
#include "button.h"
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
    else if (is_between(self->pin, PIN_GROUP_BOARD, PIN_GROUP_BOARD_END)) {
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
    if ( (self->mode & HOLD) && !(self->mode & DOUBLE)) self->handle_hold(self);
    if (!(self->mode & HOLD) &&  (self->mode & DOUBLE)) self->handle_double(self);
    if ( (self->mode & HOLD) &&  (self->mode & DOUBLE)) self->handle_hold_double(self);
    if (self->mode == STICKY) self->handle_sticky(self);
}

void Button__handle_normal(Button *self) {
    uint64_t debounce = self->press_timestamp + (CFG_PRESS_DEBOUNCE * 1000);
    if (time_us_64() < debounce) return;
    bool pressed = self->is_pressed(self);
    if(pressed && !self->state_primary) {
        hid_press_multiple(self->actions);
        self->state_primary = true;
        self->press_timestamp = time_us_64();
        return;
    }
    if((!pressed) && self->state_primary) {
        hid_release_multiple(self->actions);
        self->state_primary = false;
        return;
    }
}

void Button__handle_hold(Button *self) {
    bool immediate = self->mode & IMMEDIATE;
    uint64_t time = (self->mode & LONG) ? CFG_HOLD_LONG_TIME : CFG_HOLD_TIME;
    bool pressed = self->is_pressed(self);
    if(pressed && !self->state_primary && !self->state_secondary) {
        // Initial press.
        if (immediate) hid_press_multiple(self->actions);
        self->state_primary = true;
        self->press_timestamp = time_us_64();
        return;
    }
    if(pressed && self->state_primary && !self->state_secondary) {
        if (time_us_64() > self->press_timestamp + (time * 1000)) {
            // Pressed and being held long enough.
            hid_press_multiple(self->actions_secondary);
            if (!immediate) self->state_primary = false;
            self->state_secondary = true;
        }
    }
    if(!pressed && self->state_primary) {
        if (immediate) {
            // Released, immediate actions were triggered.
            hid_release_multiple(self->actions);
        } else {
            // Released, it was never condidered held.
            hid_press_multiple(self->actions);
            hid_release_multiple_later(self->actions, 100);
        }
        self->state_primary = false;
        return;
    }
    if(!pressed && self->state_secondary) {
        // Relased and it was condidered held.
        hid_release_multiple(self->actions_secondary);
        self->state_secondary = false;
    }
}

void Button__handle_double(Button *self) {
    bool immediate = self->mode & IMMEDIATE;
    uint16_t time = CFG_DOUBLE_PRESS_TIME;
    bool pressed = self->is_pressed(self);
    if (pressed && !self->timestamps_updated) {
        self->press_timestamp_prev = self->press_timestamp;
        self->press_timestamp = time_us_64();
        self->timestamps_updated = true;
    }
    if (!pressed) {
        self->timestamps_updated = false;
    }
    if(pressed && !self->state_terciary) {
        uint64_t elapsed = self->press_timestamp - self->press_timestamp_prev;
        bool is_double_press = elapsed < (time * 1000);
        if (is_double_press) {
            // The press is considered a double press.
            self->state_terciary = true;
            hid_press_multiple(self->actions_terciary);
        } else {
            // It is a first press.
            self->state_primary = true;
            if (!self->emitted_primary) {
                if (immediate) {
                    // Trigger primary immediately.
                    hid_press_multiple(self->actions);
                    self->emitted_primary = true;
                } else {
                    uint64_t timeout = time_us_64() > self->press_timestamp + (time * 1000);
                    if (timeout) {
                        // It has been held so long that the next press cannot be a double press.
                        hid_press_multiple(self->actions);
                        self->emitted_primary = true;
                    }
                }
            }
        }
        return;
    }
    if(!pressed && self->state_primary && !self->state_terciary) {
        if (self->emitted_primary) {
            // Released and primary actions were triggered.
            hid_release_multiple(self->actions);
            self->state_primary = false;
            self->emitted_primary = false;
        } else {
            uint64_t timeout = time_us_64() > self->press_timestamp + (time * 1000);
            if (timeout) {
                // Released for so long that the next press cannot be a double press.
                hid_press_multiple(self->actions);
                hid_release_multiple_later(self->actions, 100);
                self->state_primary = false;
            }
        }
    }
    if(!pressed && self->state_terciary) {
        // Released and it was a double press,
        hid_release_multiple(self->actions_terciary);
        self->state_primary = false;
        self->state_terciary = false;
    }
}

void Button__handle_hold_double(Button *self) {
    bool immediate = self->mode & IMMEDIATE;
    uint64_t hold_time = (self->mode & LONG) ? CFG_HOLD_LONG_TIME : CFG_HOLD_TIME;
    uint16_t double_time = CFG_DOUBLE_PRESS_TIME;
    bool pressed = self->is_pressed(self);
    if (pressed && !self->timestamps_updated) {
        self->press_timestamp_prev = self->press_timestamp;
        self->press_timestamp = time_us_64();
        self->timestamps_updated = true;
    }
    if (!pressed) {
        self->timestamps_updated = false;
    }
    if(pressed && !self->state_terciary) {
        uint64_t elapsed = self->press_timestamp - self->press_timestamp_prev;
        bool is_double_press = elapsed < (double_time * 1000);
        if (is_double_press) {
            // The press is considered a double press.
            self->state_terciary = true;
            hid_press_multiple(self->actions_terciary);
        } else {
            self->state_primary = true;
            if (!self->state_secondary) {
                if (immediate && !self->emitted_primary) {
                    // Trigger primary immediately.
                    hid_press_multiple(self->actions);
                    self->emitted_primary = true;
                }
                uint64_t timeout = time_us_64() > self->press_timestamp + (hold_time * 1000);
                if (timeout) {
                    // It has been held so long that is considered held.
                    hid_press_multiple(self->actions_secondary);
                    self->state_secondary = true;
                }
            }
        }
        return;
    }
    if (!pressed && self->emitted_primary) {
        // Released and primary actions (immediate) was triggered.
        hid_release_multiple(self->actions);
        self->emitted_primary = false;
    }
    if(!pressed && self->state_primary && !self->state_secondary && !self->state_terciary && !immediate) {
        uint64_t timeout = time_us_64() > self->press_timestamp + (double_time * 1000);
        if (timeout) {
            // Released for so long that the next press cannot be a double press.
            hid_press_multiple(self->actions);
            hid_release_multiple_later(self->actions, 100);
            self->state_primary = false;
        }
    }
    if(!pressed && self->state_secondary) {
        // Released and it was considered held.
        hid_release_multiple(self->actions_secondary);
        self->state_primary = false;
        self->state_secondary = false;
    }
    if(!pressed && self->state_terciary) {
        // Released and it was a double press.
        hid_release_multiple(self->actions_terciary);
        self->state_primary = false;
        self->state_terciary = false;
    }
}

void Button__handle_sticky(Button *self) {
    bool pressed = self->is_pressed(self);
    if(pressed && !self->state_primary) {
        self->state_primary = true;
        hid_press_multiple(self->actions);
        hid_press_multiple(self->actions_secondary);
        return;
    }
    if((!pressed) && self->state_primary) {
        self->state_primary = false;
        hid_release_multiple(self->actions_secondary);
        return;
    }
}

void Button__reset(Button *self) {
    self->state_primary = false;
    self->state_secondary = false;
    self->state_terciary = false;
}

// Init.
Button Button_ (
    uint8_t pin,
    ButtonMode mode,
    Actions actions,
    Actions actions_secondary,
    Actions actions_terciary
) {
    if (pin) {
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_IN);
        gpio_pull_up(pin);
    }
    Button button;
    memcpy(button.actions, actions, 4);
    memcpy(button.actions_secondary, actions_secondary, 4);
    memcpy(button.actions_terciary, actions_terciary, 4);
    button.is_pressed = Button__is_pressed;
    button.report = Button__report;
    button.reset = Button__reset;
    button.handle_normal = Button__handle_normal;
    button.handle_hold = Button__handle_hold;
    button.handle_double = Button__handle_double;
    button.handle_hold_double = Button__handle_hold_double;
    button.handle_sticky = Button__handle_sticky;
    button.pin = pin;
    button.mode = mode;
    button.state_primary = false;
    button.state_secondary = false;
    button.state_terciary = false;
    button.emitted_primary = false;
    button.virtual_press = false;
    button.press_timestamp = 0;
    button.press_timestamp_prev = 0;
    button.timestamps_updated = false;
    return button;
}

// Alternative init.
Button Button_from_ctrl(uint8_t pin, CtrlSection section) {
    return Button_(
        pin,
        section.button.mode,
        section.button.actions,
        section.button.actions_secondary,
        section.button.actions_terciary
    );
}
