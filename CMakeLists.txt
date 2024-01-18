# SPDX-License-Identifier: GPL-2.0-only
# Copyright (C) 2022, Input Labs Oy.

cmake_minimum_required(VERSION 3.16)

if (EXISTS ./deps/pico-sdk AND EXISTS ./deps/arm-toolchain)
    # If the dependencies are local.
    set(PICO_SDK_PATH ${CMAKE_CURRENT_SOURCE_DIR}/deps/pico-sdk)
    set(PICO_TOOLCHAIN_PATH ${CMAKE_CURRENT_SOURCE_DIR}/deps/arm-toolchain)
    include(${PICO_SDK_PATH}/pico_sdk_init.cmake)
else()
    # If the dependencies are global.
    include(scripts/pico_sdk_import.cmake)
endif()

set(PROJECT alpakka)
project(${PROJECT} C CXX ASM)
pico_sdk_init()

set(PICO_BOARD pico_w)
set(PICO_CYW43_ARCH_THREADSAFE_BACKGROUND=1)

add_executable(${PROJECT}
    src/main.c
)

target_compile_definitions(${PROJECT} INTERFACE
        WANT_HCI_DUMP=1 # enable btstack debugging
)

target_link_libraries(${PROJECT}
    pico_stdlib
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
    pico_cyw43_arch_none
    pico_btstack_cyw43
    pico_btstack_classic
    pico_btstack_ble  
)

target_include_directories(${PROJECT} PUBLIC
    src
    src/headers
)

target_sources(${PROJECT} PUBLIC
    src/bus.c
    src/button.c
    src/common.c
    src/config.c
    src/ctrl.c
    src/dhat.c
    src/glyph.c
    src/gyro.c
    src/hid.c
    src/imu.c
    src/led.c
    src/logging.c
    src/nvm.c
    src/profile.c
    src/profiles/console_legacy.c
    src/profiles/console.c
    src/profiles/custom.c
    src/profiles/desktop.c
    src/profiles/flight.c
    src/profiles/fps_fusion.c
    src/profiles/fps_wasd.c
    src/profiles/home.c
    src/profiles/racing.c
    src/profiles/rts.c
    src/self_test.c
    src/rotary.c
    src/thumbstick.c
    src/touch.c
    src/tusb_config.c
    src/bt_config.c
    src/uart.c
    src/vector.c
    src/webusb.c
    src/xinput.c
)

pico_enable_stdio_uart(${PROJECT} 1)
pico_enable_stdio_usb(${PROJECT} 0)

pico_add_extra_outputs(${PROJECT})
