// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#pragma once

typedef uint8_t Glyph[5];

uint8_t glyph_encode(Glyph glyph);
void glyph_decode(Glyph glyph, uint8_t encoded);
