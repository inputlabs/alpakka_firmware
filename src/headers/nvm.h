// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#pragma once
#include <pico/stdlib.h>

void nvm_read(uint32_t addr, uint8_t* buffer, uint32_t size);
void nvm_write(uint32_t addr, uint8_t* buffer, uint32_t size);
