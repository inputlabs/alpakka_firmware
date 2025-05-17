// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#pragma once
#include "common.h"
#include "button.h"
#include "glyph.h"

#define THUMBSTICK_BASELINE_SATURATION 1.65
#define THUMBSTICK_INNER_RADIUS 0.75
#define THUMBSTICK_ADDITIONAL_DEADZONE_FOR_BUTTONS 0.05
// #define ADC_SMOOTH_MAX 8
// #define ADC_SMOOTH_THRESHOLD 0.1

typedef enum ThumbstickMode_enum {
    THUMBSTICK_MODE_OFF,
    THUMBSTICK_MODE_4DIR,
    THUMBSTICK_MODE_ALPHANUMERIC,
    THUMBSTICK_MODE_8DIR,
} ThumbstickMode;

typedef enum ThumbstickDistance_enum {
    THUMBSTICK_DISTANCE_AXIAL,
    THUMBSTICK_DISTANCE_RADIAL,
} ThumbstickDistance;

typedef struct ThumbstickPosition_struct {
    float x;
    float y;
    float angle;
    float radius;
} ThumbstickPosition;

typedef enum Dir4Mask_enum {
    DIR4_MASK_LEFT = 1,
    DIR4_MASK_RIGHT = 2,
    DIR4_MASK_UP = 4,
    DIR4_MASK_DOWN = 8,
} Dir4Mask;

typedef enum Dir4_enum {
    DIR4_NONE,
    DIR4_LEFT,
    DIR4_RIGHT,
    DIR4_UP,
    DIR4_DOWN,
} Dir4;

typedef enum Dir8_enum {
    DIR8_CENTER,
    DIR8_LEFT,
    DIR8_RIGHT,
    DIR8_UP,
    DIR8_DOWN,
    DIR8_UP_LEFT,
    DIR8_UP_RIGHT,
    DIR8_DOWN_LEFT,
    DIR8_DOWN_RIGHT,
} Dir8;

typedef struct Thumbstick_struct Thumbstick;
struct Thumbstick_struct {
    void (*report) (Thumbstick *self);
    void (*report_4dir_axial) (Thumbstick *self, ThumbstickPosition pos);
    void (*report_4dir_radial) (Thumbstick *self, ThumbstickPosition pos);
    void (*report_8dir) (Thumbstick *self, ThumbstickPosition pos);
    void (*report_alphanumeric) (Thumbstick *self, ThumbstickPosition pos);
    void (*report_glyphstick) (Thumbstick *self, Glyph input);
    void (*report_daisywheel) (Thumbstick *self, Dir8 dir);
    void (*reset) (Thumbstick *self);
    void (*config_4dir) (Thumbstick *self, Button left, Button right, Button up, Button down, Button push, Button inner, Button outer);
    void (*config_8dir) (Thumbstick *self, Button left, Button right, Button up, Button down, Button ul, Button ur, Button dl, Button dr, Button push);
    void (*config_glyphstick) (Thumbstick *self, Actions actions, Glyph glyph);
    void (*config_daisywheel) (Thumbstick *self, uint8_t dir, uint8_t button, Actions actions);
    uint8_t index;
    uint8_t pin_x;
    uint8_t pin_y;
    bool invert_x;
    bool invert_y;
    ThumbstickMode mode;
    ThumbstickDistance distance_mode;
    bool deadzone_override;
    float deadzone;
    float antideadzone;
    float overlap;
    float saturation;
    Button left;
    Button right;
    Button up;
    Button down;
    Button ul;
    Button ur;
    Button dl;
    Button dr;
    Button push;
    Button inner;
    Button outer;
    Glyph glyphstick_glyphs[44];
    Actions glyphstick_actions[44];
    uint8_t glyphstick_index;
    Actions daisywheel[8][4];
};

Thumbstick Thumbstick_ (
    uint8_t index,
    uint8_t pin_x,
    uint8_t pin_y,
    bool invert_x,
    bool invert_y,
    ThumbstickMode mode,
    ThumbstickDistance distance_mode,
    bool deadzone_override,
    float deadzone,
    float antideadzone,
    float overlap,
    float saturation
);

void thumbstick_init();
void thumbstick_report();
void thumbstick_calibrate();
void thumbstick_update_deadzone();
void thumbstick_from_ctrl(Thumbstick *thumbstick, CtrlProfile *ctrl, uint8_t index);
