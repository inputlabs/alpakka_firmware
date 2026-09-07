// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include "loop.h"

int main() {
    #if defined DEVICE_IS_CONTROLLER
        loop_controller_init();
    #elif defined DEVICE_DONGLE
        loop_dongle_init();
    #elif defined DEVICE_LLAMA
        loop_llama_init();
    #endif
}
