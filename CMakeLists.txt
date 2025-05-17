# SPDX-License-Identifier: GPL-2.0-only
# Copyright (C) 2022, Input Labs Oy.

cmake_minimum_required(VERSION 3.16)

if (EXISTS ./deps/pico-sdk AND EXISTS ./deps/arm-toolchain)
    # If the dependencies are local.
    message("Using local dependencies")
    set(PICO_SDK_PATH ${CMAKE_CURRENT_SOURCE_DIR}/deps/pico-sdk)
    set(PICO_SDK_EXTRAS ${CMAKE_CURRENT_SOURCE_DIR}/deps/pico-extras)
    set(picotool_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps/picotool/build/bin/picotool)
    set(PICO_TOOLCHAIN_PATH ${CMAKE_CURRENT_SOURCE_DIR}/deps/arm-toolchain)
    include(${PICO_SDK_PATH}/pico_sdk_init.cmake)
    include(${PICO_SDK_EXTRAS}/external/pico_extras_import.cmake)
else()
    # If the dependencies are global.
    message("Using global dependencies")
    include(scripts/pico_sdk_import.cmake)
endif()

set(PROJECT alpakka)
set(PICO_BOARD pico)

project(${PROJECT} C CXX ASM)
pico_sdk_init()

add_executable(${PROJECT}
    src/main.c
)

if(DEFINED DEVICE)
    if(DEVICE MATCHES "alpakka_v0")
        target_compile_definitions(${PROJECT} PUBLIC DEVICE_ALPAKKA_V0=1)
        target_compile_definitions(${PROJECT} PUBLIC DEVICE_IS_ALPAKKA=1)
    elseif(DEVICE MATCHES "alpakka_v1")
        target_compile_definitions(${PROJECT} PUBLIC DEVICE_ALPAKKA_V1=1)
        target_compile_definitions(${PROJECT} PUBLIC DEVICE_IS_ALPAKKA=1)
        target_compile_definitions(${PROJECT} PUBLIC DEVICE_HAS_MARMOTA=1)
    elseif(DEVICE MATCHES "dongle")
        target_compile_definitions(${PROJECT} PUBLIC DEVICE_DONGLE=1)
        target_compile_definitions(${PROJECT} PUBLIC DEVICE_HAS_MARMOTA=1)
    elseif(DEVICE MATCHES "llama")
        target_compile_definitions(${PROJECT} PUBLIC DEVICE_LLAMA=1)
        target_compile_definitions(${PROJECT} PUBLIC DEVICE_HAS_MARMOTA=1)
    else()
        message(FATAL_ERROR "Target device unknown")
    endif()
else()
    message(FATAL_ERROR "Target device not defined")
endif()

target_link_libraries(${PROJECT} PRIVATE
    pico_stdlib
    pico_time
    pico_unique_id
    pico_bootrom
    pico_bootsel_via_double_reset
    pico_rand
    hardware_adc
    hardware_flash
    hardware_i2c
    hardware_pwm
    hardware_spi
    hardware_sync
    hardware_timer
    hardware_sleep
    tinyusb_device
)

target_include_directories(${PROJECT} PUBLIC
    src
    src/headers
    deps
)

target_sources(${PROJECT} PUBLIC
    src/bus.c
    src/button.c
    src/common.c
    src/config.c
    src/ctrl.c
    src/dhat.c
    src/esp.c
    src/glyph.c
    src/gyro.c
    src/hid.c
    src/imu.c
    src/led.c
    src/logging.c
    src/loop.c
    src/nvm.c
    src/power.c
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
    src/rotary.c
    src/self_test.c
    src/thanks.c
    src/thumbstick.c
    src/touch.c
    src/tusb_config.c
    src/uart.c
    src/vector.c
    src/webusb.c
    src/wireless.c
    src/xinput.c
)

if(DEVICE MATCHES "llama")
    set(PORT PI_PICO)
    set(MD5_ENABLED 1)  # For after-flashing hash verification.
    add_subdirectory("deps/esp-serial-flasher" ${CMAKE_BINARY_DIR}/flasher)
    target_link_libraries(${PROJECT} PRIVATE flasher)
    target_sources(${PROJECT} PUBLIC deps/esp-serial-flasher/examples/common/example_common.c)
endif()

pico_enable_stdio_uart(${PROJECT} 1)
pico_add_extra_outputs(${PROJECT})
