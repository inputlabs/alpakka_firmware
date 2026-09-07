// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

/*
Pin numbers from 0 to 29 are RP2040 GPIOs.
Pin numbers from 100 to 115 are IO Expander 1, from pin 0 to pin 15.
Pin numbers from 200 to 215 are IO Expander 2, from pin 0 to pin 15.
*/

#pragma once

// Groups.
#define PIN_GROUP_BOARD 1
#define PIN_GROUP_BOARD_END 99
#define PIN_GROUP_IO_0 100
#define PIN_GROUP_IO_0_END 199
#define PIN_GROUP_IO_1 200
#define PIN_GROUP_IO_1_END 249
#define PIN_ADC_FIRST 26

// Alpakka 0.
#ifdef DEVICE_ALPAKKA_V0
    #define PIN_FUNC 0
    #define PIN_LED_UP 2
    #define PIN_LED_LEFT 3
    #define PIN_LED_DOWN 4
    #define PIN_LED_RIGHT 5
    #define PIN_TOUCH_OUT 6
    #define PIN_TOUCH_IN 7
    #define PIN_ROTARY_B 8
    #define PIN_ROTARY_A 9
    #define PIN_SPI_CK 10
    #define PIN_SPI_TX 11
    #define PIN_SPI_RX 12
    #define PIN_I2C_SDA 14
    #define PIN_I2C_SCL 15
    #define PIN_SPI_CS0 18
    #define PIN_SPI_CS1 19
    #define PIN_HOME 20
    #define PIN_LED_BOARD 25
    #define PIN_THUMBSTICK_LY 26
    #define PIN_THUMBSTICK_LX 27
    #define PIN_THUMBSTICK_RY 0
    #define PIN_THUMBSTICK_RX 0
    #define PIN_THUMBSTICK_LX_INVERT 0
    #define PIN_THUMBSTICK_LY_INVERT 0
    #define PIN_THUMBSTICK_RX_INVERT 0
    #define PIN_THUMBSTICK_RY_INVERT 0
    // Expanders.
    #define PIN_SELECT_1 114
    #define PIN_SELECT_2 113
    #define PIN_DPAD_LEFT 104
    #define PIN_DPAD_RIGHT 101
    #define PIN_DPAD_UP 103
    #define PIN_DPAD_DOWN 100
    #define PIN_L1 102
    #define PIN_L2 115
    #define PIN_L3 112
    #define PIN_L4 109
    #define PIN_PCBGEN_0 111
    #define PIN_PCBGEN_1 110
    #define PIN_START_1 200
    #define PIN_START_2 201
    #define PIN_A 215
    #define PIN_B 210
    #define PIN_X 213
    #define PIN_Y 211
    #define PIN_DHAT_LEFT 203
    #define PIN_DHAT_RIGHT 205
    #define PIN_DHAT_UP 206
    #define PIN_DHAT_DOWN 202
    #define PIN_R1 212
    #define PIN_R2 214
    #define PIN_R3 204
    #define PIN_R4 207
#endif

// Alpakka 1.
#ifdef DEVICE_ALPAKKA_V1
    #define PIN_LED_LEFT 6
    #define PIN_LED_DOWN 7
    #define PIN_LED_RIGHT 8
    #define PIN_LED_UP 9
    #define PIN_ROTARY_B 10
    #define PIN_ROTARY_A 11
    #define PIN_TOUCH_IN 12
    #define PIN_TOUCH_OUT 13
    #define PIN_HOME 24
    #define PIN_THUMBSTICK_RY 26
    #define PIN_THUMBSTICK_RX 27
    #define PIN_THUMBSTICK_LX 29
    #define PIN_THUMBSTICK_LY 28
    #define PIN_THUMBSTICK_LX_INVERT 1
    #define PIN_THUMBSTICK_LY_INVERT 1
    #define PIN_THUMBSTICK_RX_INVERT 0
    #define PIN_THUMBSTICK_RY_INVERT 0
    // Expanders.
    #define PIN_SELECT_1 114
    #define PIN_SELECT_2 110
    #define PIN_DPAD_LEFT 104
    #define PIN_DPAD_RIGHT 101
    #define PIN_DPAD_UP 103
    #define PIN_DPAD_DOWN 100
    #define PIN_L1 102
    #define PIN_L2 115
    #define PIN_L3 109
    #define PIN_L4 108
    #define PIN_PCBGEN_0 111
    #define PIN_PCBGEN_1 113
    #define PIN_START_1 200
    #define PIN_START_2 201
    #define PIN_A 215
    #define PIN_B 210
    #define PIN_X 213
    #define PIN_Y 211
    #define PIN_R1 212
    #define PIN_R2 214
    #define PIN_R3 202
    #define PIN_R4 207
    #define PIN_DHAT_LEFT PIN_NONE
    #define PIN_DHAT_RIGHT PIN_NONE
    #define PIN_DHAT_UP PIN_NONE
    #define PIN_DHAT_DOWN PIN_NONE
#endif

#ifdef DEVICE_KAPYBARA
    #define PIN_LED_LEFT 6
    #define PIN_LED_DOWN 7
    #define PIN_LED_RIGHT 8
    #define PIN_LED_UP 9
    #define PIN_ROTARY_B 13
    #define PIN_ROTARY_A 12
    #define PIN_TOUCH_IN 11
    #define PIN_TOUCH_OUT 10
    #define PIN_HOME 24
    #define PIN_THUMBSTICK_RY 27
    #define PIN_THUMBSTICK_RX 26
    #define PIN_THUMBSTICK_LX 28
    #define PIN_THUMBSTICK_LY 29
    #define PIN_THUMBSTICK_LX_INVERT 1
    #define PIN_THUMBSTICK_LY_INVERT 0
    #define PIN_THUMBSTICK_RX_INVERT 1
    #define PIN_THUMBSTICK_RY_INVERT 0
    // Expanders.
    #define PIN_SELECT_1 102
    #define PIN_SELECT_2 103
    #define PIN_START_1 115
    #define PIN_START_2 114
    #define PIN_A 106
    #define PIN_B 113
    #define PIN_X 104
    #define PIN_Y 107
    #define PIN_L1 108
    #define PIN_L2 101
    #define PIN_L3 109
    #define PIN_R1 112
    #define PIN_R2 100
    #define PIN_R3 110
    #define PIN_DPAD_LEFT PIN_NONE
    #define PIN_DPAD_RIGHT PIN_NONE
    #define PIN_DPAD_UP PIN_NONE
    #define PIN_DPAD_DOWN PIN_NONE
    #define PIN_L4 PIN_NONE
    #define PIN_R4 PIN_NONE
    #define PIN_DHAT_LEFT PIN_NONE
    #define PIN_DHAT_RIGHT PIN_NONE
    #define PIN_DHAT_UP PIN_NONE
    #define PIN_DHAT_DOWN PIN_NONE
    #define PIN_PCBGEN_0 PIN_NONE
    #define PIN_PCBGEN_1 PIN_NONE
#endif

#ifdef DEVICE_DONGLE
    #define PIN_A PIN_NONE
    #define PIN_B PIN_NONE
    #define PIN_X PIN_NONE
    #define PIN_Y PIN_NONE
    #define PIN_L1 PIN_NONE
    #define PIN_L2 PIN_NONE
    #define PIN_L3 PIN_NONE
    #define PIN_L4 PIN_NONE
    #define PIN_R1 PIN_NONE
    #define PIN_R2 PIN_NONE
    #define PIN_R3 PIN_NONE
    #define PIN_R4 PIN_NONE
    #define PIN_LED_LEFT PIN_NONE
    #define PIN_LED_DOWN PIN_NONE
    #define PIN_LED_RIGHT PIN_NONE
    #define PIN_LED_UP PIN_NONE
    #define PIN_DHAT_LEFT PIN_NONE
    #define PIN_DHAT_RIGHT PIN_NONE
    #define PIN_DHAT_UP PIN_NONE
    #define PIN_DHAT_DOWN PIN_NONE
    #define PIN_ROTARY_B PIN_NONE
    #define PIN_ROTARY_A PIN_NONE
    #define PIN_TOUCH_IN PIN_NONE
    #define PIN_TOUCH_OUT PIN_NONE
#endif

// All devices with Marmota.
#ifdef DEVICE_HAS_MARMOTA
    #define PIN_FUNC 2
    #define PIN_LED_BOARD 3
    #define PIN_SPI_CS0 4
    #define PIN_SPI_CS1 5
    #define PIN_BATT_STAT_1 14
    #define PIN_DC_POWER_SAVE 15
    #define PIN_ESP_BOOT 17
    #define PIN_SPI_RX 16
    #define PIN_SPI_CK 18
    #define PIN_SPI_TX 19
    #define PIN_UART1_TX 20
    #define PIN_UART1_RX 21
    #define PIN_I2C_SDA 22
    #define PIN_I2C_SCL 23
    #define PIN_ESP_ENABLE 25
#endif

// Special pins.
#define PIN_GROUP_SPECIAL 250
#define PIN_NONE 255
#define PIN_VIRTUAL 254  // Buttons without any hardware associated to them.
