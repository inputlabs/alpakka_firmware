// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include <pico/stdlib.h>
#include "pico/rand.h"

uint32_t bin(uint8_t k) {
    return (k == 0 || k == 1 ? k : ((k % 2) + 10 * bin(k / 2)));
}

uint32_t bin16(uint16_t k) {
    return (k == 0 || k == 1 ? k : ((k % 2) + 10 * bin(k / 2)));
}

uint8_t random8() {
    return (uint8_t)get_rand_32();
}

void print_array(uint8_t *array, uint8_t len) {
    printf("[");
    for(uint8_t i=0; i<len; i++) printf("%i ", array[i]);
    printf("]\n");
}
