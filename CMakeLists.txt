# SPDX-License-Identifier: GPL-2.0-only
# Copyright (C) 2022, Input Labs Oy.

cmake_minimum_required(VERSION 3.16)

if (EXISTS ./deps/pico-sdk AND EXISTS ./deps/arm-toolchain)
    set(PICO_SDK_PATH ./deps/pico-sdk)
    set(PICO_TOOLCHAIN_PATH ./deps/arm-toolchain)
    include(${PICO_SDK_PATH}/pico_sdk_init.cmake)
else()
    include(pico_sdk_import.cmake)
endif()

set(PROJECT alpakka)
project(${PROJECT} C CXX ASM)
pico_sdk_init()

add_executable(${PROJECT}
    src/main.c
)

target_link_libraries(${PROJECT}
    pico_stdlib
    pico_multicore
    pico_time
    pico_unique_id
    pico_bootrom
    pico_bootsel_via_double_reset
    hardware_adc
    hardware_flash
    hardware_i2c
    hardware_pwm
    hardware_spi
    hardware_sync
    hardware_timer
    tinyusb_device
)

target_include_directories(${PROJECT} PUBLIC
    src
    src/headers
)

target_sources(${PROJECT} PUBLIC
    src/bus.c
    src/button.c
    src/config.c
    src/dhat.c
    src/gyro.c
    src/helper.c
    src/hid.c
    src/imu.c
    src/led.c
    src/nvm.c
    src/profile.c
    src/profiles/console_legacy.c
    src/profiles/console.c
    src/profiles/desktop.c
    src/profiles/fps_fusion.c
    src/profiles/fps_wasd.c
    src/profiles/home.c
    src/profiles/none.c
    src/rotary.c
    src/thumbstick.c
    src/touch.c
    src/tusb_config.c
    src/uart.c
    src/xinput.c
)

pico_enable_stdio_uart(${PROJECT} 1)
pico_add_extra_outputs(${PROJECT})
