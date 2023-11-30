// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#pragma once

typedef u8 Glyph[5];

u8 glyph_encode(Glyph glyph);
void glyph_decode(Glyph glyph, u8 encoded);
