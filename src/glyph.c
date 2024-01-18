// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

/*
In the alphanumeric input (keyboard emulation) implemented in the thumbstick,
a series of "glyphs" are mapped into specific key presses. These glyphs are a
sequence of 1 to 5 cardinal directions directions.
When packing this information into the Ctrl protocol we want to be as space
efficient as possible, and encode/decode a glyph into a single 8bit number.
*/

#include "common.h"
#include "glyph.h"
#include "thumbstick.h"

// Encode a sequence of a maximum of 5 directions into a single uint8.
uint8_t glyph_encode(Glyph glyph) {
    uint8_t encoded = 0;
    uint8_t len = 5;
    // Determine length.
    if      (glyph[1] == 0) len = 1;
    else if (glyph[2] == 0) len = 2;
    else if (glyph[3] == 0) len = 3;
    else if (glyph[4] == 0) len = 4;
    // Initial direction. Shifted by one because 2 bits should be able to
    // represent the 4 directions, and thus skipping DIR4_NONE.
    encoded += glyph[0] - 1;
    // Termination bit.
    encoded += 1 << 1+len;
    // Remaining directions.
    for(uint8_t i=1; i<len; i++) {
        // Each subsequent direction is either clockwise or anticlockwise, so
        // it is encoded in a single bit.
        if (glyph[i-1] == DIR4_UP    && glyph[i] == DIR4_RIGHT) encoded += (1 << 1+i);
        if (glyph[i-1] == DIR4_RIGHT && glyph[i] == DIR4_DOWN)  encoded += (1 << 1+i);
        if (glyph[i-1] == DIR4_DOWN  && glyph[i] == DIR4_LEFT)  encoded += (1 << 1+i);
        if (glyph[i-1] == DIR4_LEFT  && glyph[i] == DIR4_UP)    encoded += (1 << 1+i);
    }
    return encoded;
}

// Decode a sequence of a maximum 5 directions from a single uint8.
void glyph_decode(Glyph glyph, uint8_t encoded) {
    uint8_t len;
    // Determine length.
    if (encoded >> 2 == 1) len = 1;
    if (encoded >> 3 == 1) len = 2;
    if (encoded >> 4 == 1) len = 3;
    if (encoded >> 5 == 1) len = 4;
    if (encoded >> 6 == 1) len = 5;
    // Initial direction. Shifted by one because 2 bits should be able to
    // represent the 4 directions, and thus skipping DIR4_NONE.
    glyph[0] = (encoded & 0b00000011) + 1;
    // Remaining directions.
    for(uint8_t i=1; i<len; i++) {
        if (encoded & (1<<i+1)) {
            // Clockwise.
            if (glyph[i-1] == DIR4_UP)    glyph[i] = DIR4_RIGHT;
            if (glyph[i-1] == DIR4_RIGHT) glyph[i] = DIR4_DOWN;
            if (glyph[i-1] == DIR4_DOWN)  glyph[i] = DIR4_LEFT;
            if (glyph[i-1] == DIR4_LEFT)  glyph[i] = DIR4_UP;
        } else {
            // Anti-clockwise.
            if (glyph[i-1] == DIR4_UP)    glyph[i] = DIR4_LEFT;
            if (glyph[i-1] == DIR4_RIGHT) glyph[i] = DIR4_UP;
            if (glyph[i-1] == DIR4_DOWN)  glyph[i] = DIR4_RIGHT;
            if (glyph[i-1] == DIR4_LEFT)  glyph[i] = DIR4_DOWN;
        }
    }
}
