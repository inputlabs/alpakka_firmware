// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include "loop.h"

int main() {
    #ifdef FW_DEVICE_ALPAKKA
        loop_device_init();
    #endif
    #ifdef FW_DEVICE_DONGLE
        loop_host_init();
    #endif
}
