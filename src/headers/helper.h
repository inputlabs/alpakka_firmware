// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#pragma once
#include <pico/stdlib.h>
#include <math.h>

#define min(a, b)  ((a < b) ? a : b)
#define max(a, b)  ((a > b) ? a : b)
#define limit_between(value, low, high)  max(low, min(high, value))
#define is_between(value, low, high)  value == limit_between(value, low, high)
#define degrees(radians)  (radians * 180.0 / M_PI)
#define radians(degrees)  (degrees * M_PI / 180.0)
#define ramp_low(x, z)  x<z ? 0 : (x-z) * (1 / (1-z))
#define ramp_mid(x, z)  x<z ? 0 : x>(1-z) ? 1 : (x-z) * (1 / (1 - 2*z))

uint32_t bin(uint8_t k);
uint32_t bin16(uint16_t k);
uint8_t random8();
