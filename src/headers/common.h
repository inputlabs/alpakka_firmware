// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#pragma once
#include <pico/stdlib.h>
#include <math.h>

#define BIT_18 262143
#define BIT_17 131071
#define BIT_16 65535
#define BIT_15 32767
#define BIT_14 16383
#define BIT_12 4095
#define BIT_11 2047
#define BIT_10 1023
#define BIT_8 255
#define BIT_7 127

#define min(a, b)  ((a < b) ? a : b)
#define max(a, b)  ((a > b) ? a : b)
#define constrain(value, low, high)  max(low, min(high, value))
#define is_between(value, low, high)  ( (value >= low) && (value <= high) )
#define sign(value)  ( value >= 0 ? 1 : -1 )
#define smooth(smoothed, value, factor)  ( (smoothed*factor + value) / (factor+1) )

#define degrees(radians)  ( radians * 180.0 / M_PI )
#define radians(degrees)  ( degrees * M_PI / 180.0 )

uint32_t bin(uint8_t k);
uint32_t bin16(uint16_t k);
uint8_t random8();
void print_array(uint8_t *array, uint8_t len);

// Ramp function that results in "deadzone" in the lower part of the range.
// (Not suited for negative K values).
// Input X: X axis as unit value.
// Input K: Factor as unit value.
// Original: (0,0)->(1,1)
// Result:   (0,0)->(k,0)->(1,1)
#define ramp_low(x, k)  ( (x-k) * (1 / (1-k)) )

// Ramp function that results in "anti-deadzone" in the lower part of the range.
// (Suited only for negative K values).
// Input X: X axis as unit value.
// Input K: Factor as unit value.
// Original: (0,0)->(1,1)
// Result:   (0,0)->(0,k)->(1,1)
#define ramp_inv(x, k)  ( 1 + ((x-1) * (1-k)) )

// Ramp function that results in "deadzone" in both the lower and the upper
// part of the range.
// (Not suited for negative K values).
// Input X: X axis as unit value.
// Input K: Factor as unit value.
// Original: (0,0)->(1,1)
// Result:   (0,0)->(k,0)->(0.5,0.5)->(1-k,1)->(1,1)
#define ramp_mid(x, z)  ( x<z ? 0 : x>(1-z) ? 1 : (x-z) * (1 / (1 - 2*z)) )

// Returns a value from -1 to 1 that represents the relative value position
// in relation to the given minimum and maximum.
// Input X: Arbitrary value (in any range).
// Input MIN: Lower part of the range, that will result in -1.
// Input MAX: Upper part of the range, that will result in +1.
#define ramp(x, min, max)  constrain( 2 * ((x-min) / (max-min)) - 1, -1, 1)

void print_array(uint8_t *array, uint8_t len);
