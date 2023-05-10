// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#pragma once
#include "button.h"

#define ANALOG_FACTOR 32767
#define TRIGGER_FACTOR 255
#define DEADZONE_FROM_CONFIG -1
#define GLYPH(...)  __VA_ARGS__, SENTINEL

typedef enum ThumbstickMode_enum {
    THUMBSTICK_MODE_OFF,
    THUMBSTICK_MODE_4DIR,
    THUMBSTICK_MODE_ALPHANUMERIC,
} ThumbstickMode;

typedef struct ThumbstickPosition_struct {
    float x;
    float y;
    float angle;
    float radius;
} ThumbstickPosition;

typedef enum Dir4_enum {
    DIR4_CENTER,
    DIR4_UP,
    DIR4_DOWN,
    DIR4_LEFT,
    DIR4_RIGHT,
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
    void (*report_4dir) (Thumbstick *self, ThumbstickPosition pos, float deadzone);
    void (*report_alphanumeric) (Thumbstick *self, ThumbstickPosition pos);
    void (*reset) (Thumbstick *self);
    void (*config_glyphstick) (Thumbstick *self, ...);
    void (*report_glyphstick) (Thumbstick *self, uint8_t len, Dir4 *input);
    void (*config_daisywheel) (Thumbstick *self, ...);
    void (*report_daisywheel) (Thumbstick *self, Dir8 dir);
    ThumbstickMode mode;
    float deadzone;
    float overlap;
    Button left;
    Button right;
    Button up;
    Button down;
    Button push;
    Button inner;
    Button outer;
    uint8_t glyphstick_glyphs[64][8];
    uint8_t glyphstick_actions[64][4];
    uint8_t daisywheel[8][4][4];
};

Thumbstick Thumbstick_ (
    ThumbstickMode mode,
    float deadzone,
    float overlap,
    Button up,
    Button left,
    Button right,
    Button down,
    Button push,
    Button inner,
    Button outer
);

void thumbstick_init();
void thumbstick_report();
void thumbstick_calibrate();
void thumbstick_update_deadzone();
