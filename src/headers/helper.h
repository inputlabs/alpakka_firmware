// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#pragma once
#include <pico/stdlib.h>
#include <math.h>

#define BIT_16 65535
#define BIT_15 32767
#define BIT_14 16383
#define BIT_11 2047
#define BIT_8 255
#define BIT_7 127

#define min(a, b)  ((a < b) ? a : b)
#define max(a, b)  ((a > b) ? a : b)
#define constrain(value, low, high)  max(low, min(high, value))
#define is_between(value, low, high)  ( (value >= low) && (value <= high) )
#define degrees(radians)  ( radians * 180.0 / M_PI )
#define radians(degrees)  ( degrees * M_PI / 180.0 )
#define ramp_low(x, z)  ( x<z ? 0 : (x-z) * (1 / (1-z)) )
#define ramp_inv(x, z)  ( x + (z * x) )
#define ramp_mid(x, z)  ( x<z ? 0 : x>(1-z) ? 1 : (x-z) * (1 / (1 - 2*z)) )
#define ramp(x, min, max)  ( x<=min ? 0 : (x>=max ? 1 : (1 * (x-min) / (max-min))) )

uint32_t bin(uint8_t k);
uint32_t bin16(uint16_t k);
uint8_t random8();
