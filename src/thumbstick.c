// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <pico/stdlib.h>
#include <hardware/adc.h>
#include "config.h"
#include "pin.h"
#include "button.h"
#include "thumbstick.h"
#include "common.h"
#include "hid.h"
#include "profile.h"
#include "logging.h"

float offset_lx = 0;
float offset_ly = 0;
float offset_rx = 0;
float offset_ry = 0;
float config_deadzone = 0;
uint8_t thumbstick_smooth_samples = 0;
float smoothed[4] = {0, 0, 0, 0};

// ============================================================================
// Static functions.

float thumbstick_adc(uint8_t pin) {
    uint8_t channel = pin - PIN_ADC_FIRST;
    adc_select_input(channel);
    float value = ((float)adc_read() - BIT_11) / BIT_11;
    return value * THUMBSTICK_BASELINE_SATURATION;
}

float thumbstick_adc_smoothed(uint8_t pin) {
    if (!thumbstick_smooth_samples) return thumbstick_adc(pin);
    uint8_t channel = pin - PIN_ADC_FIRST;
    float value = thumbstick_adc(pin);
    value = smooth(smoothed[channel], value, (float)(thumbstick_smooth_samples));  // Rolling average.
    smoothed[channel] = value;
    return value;
}

void thumbstick_update_deadzone() {
    uint8_t preset = config_get_deadzone_preset();
    config_deadzone = config_get_deadzone_value(preset);
}

void thumbstick_update_offsets() {
    Config *config = config_read();
    offset_lx = config->offset_ts_lx;
    offset_ly = config->offset_ts_ly;
    offset_rx = config->offset_ts_rx;
    offset_ry = config->offset_ts_ry;
}

// Refresh runtime smoothing factor with value from config.
void thumbstick_update_smooth_samples() {
    Config *config = config_read();
    thumbstick_smooth_samples = config->thumbstick_smooth_samples;
}

void thumbstick_calibrate_each(uint8_t pin_x, uint8_t pin_y, float *result_x, float *result_y) {
    info("Thumbstick: calibrating axis...\n");
    float x = 0;
    float y = 0;
    uint32_t nsamples = CFG_CALIBRATION_SAMPLES_THUMBSTICK;
    info("| 0%%%*s100%% |\n", CFG_CALIBRATION_PROGRESS_BAR - 10, "");
    for(uint32_t i=0; i<nsamples; i++) {
        x += thumbstick_adc(pin_x);
        y += thumbstick_adc(pin_y);
        if (!(i % (nsamples / CFG_CALIBRATION_PROGRESS_BAR))) info("=");
    }
    x /= CFG_CALIBRATION_SAMPLES_THUMBSTICK;
    y /= CFG_CALIBRATION_SAMPLES_THUMBSTICK;
    info("\nThumbstick: calibrated x=%.03f y=%.03f\n", x, y);
    *result_x = x;
    *result_y = y;
}

void thumbstick_calibrate() {
    float lx = 0;
    float ly = 0;
    float rx = 0;
    float ry = 0;
    thumbstick_calibrate_each(PIN_THUMBSTICK_LX, PIN_THUMBSTICK_LY, &lx, &ly);
    #ifdef DEVICE_HAS_TWO_THUMBSTICKS
        thumbstick_calibrate_each(PIN_THUMBSTICK_RX, PIN_THUMBSTICK_RY, &rx, &ry);
    #endif
    config_set_thumbstick_offset(lx, ly, rx, ry);
    thumbstick_update_offsets();
}

void thumbstick_init() {
    info("INIT: Thumbstick\n");
    adc_init();
    adc_gpio_init(PIN_THUMBSTICK_LX);
    adc_gpio_init(PIN_THUMBSTICK_LY);
    #ifdef DEVICE_HAS_TWO_THUMBSTICKS
        adc_gpio_init(PIN_THUMBSTICK_RX);
        adc_gpio_init(PIN_THUMBSTICK_RY);
    #endif
    thumbstick_update_offsets();
    thumbstick_update_deadzone();
    thumbstick_update_smooth_samples();
    thumbstick_init_daisywheel();
}

uint8_t thumbstick_get_direction(float angle, float overlap) {
    float a = 45 * (1 - overlap);
    float b = 180 - a;
    uint8_t mask = 0;
    if (is_between(angle, -b, -a)) mask += DIR4_MASK_LEFT;
    if (is_between(angle, a, b)) mask += DIR4_MASK_RIGHT;
    if (fabs(angle) <= (90 - a)) mask += DIR4_MASK_UP;
    if (fabs(angle) >= (90 + a)) mask += DIR4_MASK_DOWN;
    return mask;
}

void thumbstick_from_ctrl(Thumbstick *thumbstick, CtrlProfile *ctrl, uint8_t index) {
    const uint8_t SECTION_STICK_SETTINGS = index ? SECTION_RSTICK_SETTINGS : SECTION_LSTICK_SETTINGS;
    const uint8_t SECTION_STICK_LEFT = index ? SECTION_RSTICK_LEFT : SECTION_LSTICK_LEFT;
    const uint8_t SECTION_STICK_RIGHT = index ? SECTION_RSTICK_RIGHT : SECTION_LSTICK_RIGHT;
    const uint8_t SECTION_STICK_UP = index ? SECTION_RSTICK_UP : SECTION_LSTICK_UP;
    const uint8_t SECTION_STICK_DOWN = index ? SECTION_RSTICK_DOWN : SECTION_LSTICK_DOWN;
    const uint8_t SECTION_STICK_UL = index ? SECTION_RSTICK_UL : SECTION_LSTICK_UL;
    const uint8_t SECTION_STICK_UR = index ? SECTION_RSTICK_UR : SECTION_LSTICK_UR;
    const uint8_t SECTION_STICK_DL = index ? SECTION_RSTICK_DL : SECTION_LSTICK_DL;
    const uint8_t SECTION_STICK_DR = index ? SECTION_RSTICK_DR : SECTION_LSTICK_DR;
    const uint8_t SECTION_STICK_PUSH = index ? SECTION_RSTICK_PUSH : SECTION_LSTICK_PUSH;
    const uint8_t SECTION_STICK_INNER = index ? SECTION_RSTICK_INNER : SECTION_LSTICK_INNER;
    const uint8_t SECTION_STICK_OUTER = index ? SECTION_RSTICK_OUTER : SECTION_LSTICK_OUTER;
    const uint8_t PIN_PUSH = index ? PIN_R3 : PIN_L3;

    CtrlThumbstick ctrl_thumbstick = ctrl->sections[SECTION_STICK_SETTINGS].thumbstick;
    *thumbstick = Thumbstick_(
        index,
        index==0 ? PIN_THUMBSTICK_LX : PIN_THUMBSTICK_RX,
        index==0 ? PIN_THUMBSTICK_LY : PIN_THUMBSTICK_RY,
        index==0 ? PIN_THUMBSTICK_LX_INVERT : PIN_THUMBSTICK_RX_INVERT,
        index==0 ? PIN_THUMBSTICK_LY_INVERT : PIN_THUMBSTICK_RY_INVERT,
        ctrl_thumbstick.mode,
        ctrl_thumbstick.radial_mode,
        ctrl_thumbstick.deadzone_override,
        ctrl_thumbstick.deadzone / 100.0,
        ctrl_thumbstick.antideadzone / 100.0,
        (int8_t)ctrl_thumbstick.overlap / 100.0,
        ctrl_thumbstick.saturation / 100.0,
        ctrl_thumbstick.outer_threshold / 100.0,
        (bool)ctrl_thumbstick.push_auto_toggle,
        ctrl_thumbstick.sens_mouse * CTRL_STICK_SENS_MOUSE_FACTOR,
        ctrl_thumbstick.sens_scroll,
        ctrl_thumbstick.sens_xy_ratio / 100.0,
        (int8_t)ctrl_thumbstick.accel_curve / 100.0,
        ctrl_thumbstick.rot_center_deadzone / 100.0,
        ctrl_thumbstick.rot_entry_deadzone,
        (bool)ctrl_thumbstick.rot_anticlockwise,
        (bool)ctrl_thumbstick.rot_any_angle,
        (bool)ctrl_thumbstick.rot_rws_enabled,
        ctrl_thumbstick.rot_rws * CTRL_STICK_RWS_FACTOR,
        ctrl_thumbstick.rot_sens_axis * CTRL_STICK_SENS_AXIS_FACTOR,
        ctrl_thumbstick.rot_smoothing,
        ctrl_thumbstick.rot_flick_time * CTRL_STICK_FLICK_TIME_FACTOR,
        (bool)ctrl_thumbstick.rot_keep_value
    );
    // Safe defaults.
    if (thumbstick->saturation == 0) thumbstick->saturation = 1.0;
    if (thumbstick->outer_threshold == 0) thumbstick->outer_threshold = 0.8;
    if (thumbstick->sens_xy_ratio == 0) thumbstick->sens_xy_ratio = 1.0;
    if (thumbstick->rot_rws == 0) thumbstick->rot_rws = 4.0;
    if (thumbstick->sens_mouse == 0) thumbstick->sens_mouse = 2000;
    if (thumbstick->sens_scroll == 0) thumbstick->sens_scroll = 20;
    if (thumbstick->rot_sens_axis == 0) thumbstick->rot_sens_axis = 100;
    if (thumbstick->rot_smoothing == 0) thumbstick->rot_smoothing = 10;
    if (thumbstick->rot_center_deadzone == 0) thumbstick->rot_center_deadzone = 50;
    // Modes config.
    if (ctrl_thumbstick.mode == THUMBSTICK_MODE_4DIR || ctrl_thumbstick.mode == THUMBSTICK_MODE_ROTATION) {
        thumbstick->config_4dir(
            thumbstick,
            Button_from_ctrl(PIN_VIRTUAL, ctrl->sections[SECTION_STICK_LEFT]),
            Button_from_ctrl(PIN_VIRTUAL, ctrl->sections[SECTION_STICK_RIGHT]),
            Button_from_ctrl(PIN_VIRTUAL, ctrl->sections[SECTION_STICK_UP]),
            Button_from_ctrl(PIN_VIRTUAL, ctrl->sections[SECTION_STICK_DOWN]),
            Button_from_ctrl(PIN_PUSH,    ctrl->sections[SECTION_STICK_PUSH]),
            Button_from_ctrl(PIN_VIRTUAL, ctrl->sections[SECTION_STICK_INNER]),
            Button_from_ctrl(PIN_VIRTUAL, ctrl->sections[SECTION_STICK_OUTER])
        );
    }
    if (ctrl_thumbstick.mode == THUMBSTICK_MODE_8DIR) {
        thumbstick->config_8dir(
            thumbstick,
            Button_from_ctrl(PIN_VIRTUAL,   ctrl->sections[SECTION_STICK_LEFT]),
            Button_from_ctrl(PIN_VIRTUAL,   ctrl->sections[SECTION_STICK_RIGHT]),
            Button_from_ctrl(PIN_VIRTUAL,   ctrl->sections[SECTION_STICK_UP]),
            Button_from_ctrl(PIN_VIRTUAL,   ctrl->sections[SECTION_STICK_DOWN]),
            Button_from_ctrl(PIN_VIRTUAL,   ctrl->sections[SECTION_STICK_UL]),
            Button_from_ctrl(PIN_VIRTUAL,   ctrl->sections[SECTION_STICK_UR]),
            Button_from_ctrl(PIN_VIRTUAL,   ctrl->sections[SECTION_STICK_DL]),
            Button_from_ctrl(PIN_VIRTUAL,   ctrl->sections[SECTION_STICK_DR]),
            Button_from_ctrl(PIN_PUSH,      ctrl->sections[SECTION_STICK_PUSH])
        );
    }
    if (ctrl_thumbstick.mode == THUMBSTICK_MODE_ALPHANUMERIC) {
        // Iterate sections.
        for(uint8_t s=0; s<4; s++) {
            // Iterate groups.
            for(uint8_t g=0; g<11; g++) {
                CtrlGlyph ctrl_glyph = ctrl->sections[SECTION_GLYPHS_0+s].glyphs.glyphs[g];
                Glyph glyph = {0};
                glyph_decode(glyph, ctrl_glyph.glyph);
                thumbstick->config_glyphstick(
                    thumbstick,
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
                CtrlDaisyGroup group = ctrl->sections[SECTION_DAISY_0+s].daisy.groups[g];
                thumbstick->config_daisywheel(thumbstick, dir, 0, group.actions_a);
                thumbstick->config_daisywheel(thumbstick, dir, 1, group.actions_b);
                thumbstick->config_daisywheel(thumbstick, dir, 2, group.actions_x);
                thumbstick->config_daisywheel(thumbstick, dir, 3, group.actions_y);
                dir += 1;
            }
        }
    }
}

// ============================================================================
// Class.

void Thumbstick__report(Thumbstick *self) {
    float offset_x = self->index==0 ? offset_lx : offset_rx;
    float offset_y = self->index==0 ? offset_ly : offset_ry;
    // Do not report if not calibrated.
    return; //////////// REMOVE
    if (offset_x == 0 && offset_y == 0) return;
    // Get values from ADC.
    float raw_x = thumbstick_adc_smoothed(self->pin_x) - offset_x;
    float raw_y = thumbstick_adc_smoothed(self->pin_y) - offset_y;
    float x = raw_x / self->saturation;
    float y = raw_y / self->saturation;
    x = constrain(x, -1, 1) * (self->invert_x? -1 : 1);
    y = constrain(y, -1, 1) * (self->invert_y? -1 : 1);
    // Get correct deadzone.
    float deadzone = 0;
    if (self->mode == THUMBSTICK_MODE_ROTATION) {
        deadzone = self->rot_center_deadzone;
    } else {
        deadzone = self->deadzone_override ? self->deadzone : config_deadzone;
        deadzone /= self->saturation;
    }
    // Calculate trigonometry.
    float angle = atan2(x, -y) * (180 / M_PI);
    float raw_radius = sqrt(powf(raw_x, 2) + powf(raw_y, 2));
    float radius = sqrt(powf(x, 2) + powf(y, 2));
    radius = ramp_low(radius, deadzone);  // Deadzone.
    radius = ramp_inv(radius, self->antideadzone);  // Antideadzone.
    radius = constrain(radius, 0, 1);
    radius = felix_curve(radius, self->accel_curve);  // Acceleration.
    if (raw_radius < deadzone) radius = 0;
    x = sin(radians(angle)) * radius;
    y = -cos(radians(angle)) * radius;
    y = constrain(y * self->sens_xy_ratio, -1, 1);
    ThumbstickPosition pos = {x, y, angle, radius};
    // Report.
    if (self->mode == THUMBSTICK_MODE_4DIR) {
        self->report_4dir(self, pos, raw_radius);
    }
    else if (self->mode == THUMBSTICK_MODE_8DIR) {
        self->report_8dir(self, pos, raw_radius);
    }
    else if (self->mode == THUMBSTICK_MODE_ROTATION) {
        self->report_rotation(self, pos, raw_radius);
    }
    else if (self->mode == THUMBSTICK_MODE_ALPHANUMERIC) {
        self->report_alphanumeric(self, pos);
    }
}

void Thumbstick__reset(Thumbstick *self) {
    if (self->mode == THUMBSTICK_MODE_4DIR) {
        self->left.reset(&self->left);
        self->right.reset(&self->right);
        self->up.reset(&self->up);
        self->down.reset(&self->down);
        self->push.reset(&self->push);
        self->inner.reset(&self->inner);
        self->outer.reset(&self->outer);
    }
}

Thumbstick Thumbstick_ (
    uint8_t index,
    uint8_t pin_x,
    uint8_t pin_y,
    bool invert_x,
    bool invert_y,
    ThumbstickMode mode,
    bool radial_mode,
    bool deadzone_override,
    float deadzone,
    float antideadzone,
    float overlap,
    float saturation,
    float outer_threshold,
    bool push_auto_toggle,
    float sens_mouse,
    float sens_scroll,
    float sens_xy_ratio,
    float accel_curve,
    float rot_center_deadzone,
    float rot_entry_deadzone,
    bool rot_anticlockwise,
    bool rot_any_angle,
    bool rot_rws_enabled,
    float rot_rws,
    float rot_sens_axis,
    float rot_smoothing,
    float rot_flick_time,
    bool rot_keep_value
) {
    Thumbstick thumbstick;
    // Methods.
    thumbstick.report = Thumbstick__report;
    thumbstick.report_4dir = Thumbstick__report_4dir;
    thumbstick.report_4dir_dir = Thumbstick__report_4dir_dir;
    thumbstick.report_4dir_axis = Thumbstick__report_4dir_axis;
    thumbstick.report_8dir = Thumbstick__report_8dir;
    thumbstick.report_rotation = Thumbstick__report_rotation;
    thumbstick.report_push_auto_toggle = Thumbstick__report_push_auto_toggle;
    thumbstick.report_alphanumeric = Thumbstick__report_alphanumeric;
    thumbstick.reset = Thumbstick__reset;
    thumbstick.config_4dir = Thumbstick__config_4dir;
    thumbstick.config_8dir = Thumbstick__config_8dir;
    thumbstick.config_glyphstick = Thumbstick__config_glyphstick;
    thumbstick.report_glyphstick = Thumbstick__report_glyphstick;
    thumbstick.config_daisywheel = Thumbstick__config_daisywheel;
    thumbstick.report_daisywheel = Thumbstick__report_daisywheel;
    // Attributes.
    thumbstick.index = index;
    thumbstick.pin_x = pin_x;
    thumbstick.pin_y = pin_y;
    thumbstick.invert_x = invert_x;
    thumbstick.invert_y = invert_y;
    thumbstick.mode = mode;
    thumbstick.radial_mode = radial_mode;
    thumbstick.deadzone_override = deadzone_override;
    thumbstick.deadzone = deadzone;
    thumbstick.antideadzone = antideadzone;
    thumbstick.overlap = overlap;
    thumbstick.saturation = saturation;
    thumbstick.glyphstick_index = 0;
    thumbstick.outer_threshold = outer_threshold;
    thumbstick.push_auto_toggle = push_auto_toggle;
    thumbstick.sens_mouse = sens_mouse;
    thumbstick.sens_scroll = sens_scroll;
    thumbstick.sens_xy_ratio = sens_xy_ratio;
    thumbstick.accel_curve = accel_curve;
    thumbstick.rot_center_deadzone = rot_center_deadzone;
    thumbstick.rot_entry_deadzone = rot_entry_deadzone;
    thumbstick.rot_anticlockwise = rot_anticlockwise;
    thumbstick.rot_any_angle = rot_any_angle;
    thumbstick.rot_rws_enabled = rot_rws_enabled;
    thumbstick.rot_rws = rot_rws;
    thumbstick.rot_sens_axis = rot_sens_axis;
    thumbstick.rot_smoothing = rot_smoothing;
    thumbstick.rot_flick_time = rot_flick_time;
    thumbstick.rot_keep_value = rot_keep_value;
    return thumbstick;
}
