// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include <hardware/flash.h>
#include <hardware/sync.h>
#include <pico/multicore.h>
#include "common.h"
#include "nvm.h"

void nvm_write(uint32_t addr, uint8_t* buffer, uint32_t size) {
    bool lock_other_core = multicore_lockout_victim_is_initialized(1);
    if (lock_other_core) multicore_lockout_start_blocking();
    uint32_t interrupts = save_and_disable_interrupts();
    flash_range_erase(addr, max(size, 4096));
    flash_range_program(addr, (const uint8_t*)buffer, size);
    restore_interrupts(interrupts);
    if (lock_other_core) multicore_lockout_end_blocking();
    printf("A5\n");
}

void nvm_read(uint32_t addr, uint8_t* buffer, uint32_t size) {
    uint8_t* p = (uint8_t*)(XIP_BASE + addr);
    for(uint32_t i=0; i<size; i++) {
        buffer[i] = *(p+i);
    }
}
