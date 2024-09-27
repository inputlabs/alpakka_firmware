// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#pragma once
#include <pico/time.h>
#include "common.h"

#define MODIFIER_INDEX 154
#define MOUSE_INDEX 162
#define GAMEPAD_INDEX 174
#define GAMEPAD_AXIS_INDEX 190
#define PROC_INDEX 202

#define MODIFIER_INDEX_END  MOUSE_INDEX - 1
#define MOUSE_INDEX_END  GAMEPAD_INDEX - 1
#define GAMEPAD_INDEX_END  GAMEPAD_AXIS_INDEX - 1
#define GAMEPAD_AXIS_INDEX_END  PROC_INDEX - 1
#define PROC_INDEX_END 255

#define KEY_NONE 0

#define KEY_A 4
#define KEY_B 5
#define KEY_C 6
#define KEY_D 7
#define KEY_E 8
#define KEY_F 9
#define KEY_G 10
#define KEY_H 11
#define KEY_I 12
#define KEY_J 13
#define KEY_K 14
#define KEY_L 15
#define KEY_M 16
#define KEY_N 17
#define KEY_O 18
#define KEY_P 19
#define KEY_Q 20
#define KEY_R 21
#define KEY_S 22
#define KEY_T 23
#define KEY_U 24
#define KEY_V 25
#define KEY_W 26
#define KEY_X 27
#define KEY_Y 28
#define KEY_Z 29

#define KEY_1 30
#define KEY_2 31
#define KEY_3 32
#define KEY_4 33
#define KEY_5 34
#define KEY_6 35
#define KEY_7 36
#define KEY_8 37
#define KEY_9 38
#define KEY_0 39

#define KEY_ENTER 40
#define KEY_ESCAPE 41
#define KEY_BACKSPACE 42
#define KEY_TAB 43
#define KEY_SPACE 44
#define KEY_MINUS 45
#define KEY_EQUALS 46
#define KEY_BRACKET_LEFT 47
#define KEY_BRACKET_RIGHT 48
#define KEY_BACKSLASH 49
#define KEY_SEMICOLON 51
#define KEY_QUOTE 52
#define KEY_BACKQUOTE 53
#define KEY_COMMA 54
#define KEY_PERIOD 55
#define KEY_SLASH 56
#define KEY_CAPS_LOCK 57

#define KEY_ISO_1 50
#define KEY_ISO_2 100

#define KEY_F1 58
#define KEY_F2 59
#define KEY_F3 60
#define KEY_F4 61
#define KEY_F5 62
#define KEY_F6 63
#define KEY_F7 64
#define KEY_F8 65
#define KEY_F9 66
#define KEY_F10 67
#define KEY_F11 68
#define KEY_F12 69

#define KEY_PRINT_SCREEN 70
#define KEY_SCROLL_LOCK 71
#define KEY_PAUSE 72
#define KEY_INSERT 73
#define KEY_HOME 74
#define KEY_PAGE_UP 75
#define KEY_DELETE 76
#define KEY_END 77
#define KEY_PAGE_DOWN 78

#define KEY_RIGHT 79
#define KEY_LEFT 80
#define KEY_DOWN 81
#define KEY_UP 82

#define KEY_PAD_NUMLOCK 83
#define KEY_PAD_SLASH 84
#define KEY_PAD_ASTERISK 85
#define KEY_PAD_MINUS 86
#define KEY_PAD_PLUS 87
#define KEY_PAD_ENTER 88
#define KEY_PAD_1 89
#define KEY_PAD_2 90
#define KEY_PAD_3 91
#define KEY_PAD_4 92
#define KEY_PAD_5 93
#define KEY_PAD_6 94
#define KEY_PAD_7 95
#define KEY_PAD_8 96
#define KEY_PAD_9 97
#define KEY_PAD_0 98
#define KEY_PAD_PERIOD 99

#define KEY_POWER 102

#define KEY_F13 104
#define KEY_F14 105
#define KEY_F15 106
#define KEY_F16 107
#define KEY_F17 108
#define KEY_F18 109
#define KEY_F19 110
#define KEY_F20 111
#define KEY_F21 112
#define KEY_F22 113
#define KEY_F23 114
#define KEY_F24 115

#define KEY_MUTE 127
#define KEY_VOLUME_UP 128
#define KEY_VOLUME_DOWN 129

#define KEY_KANJI_1 135
#define KEY_KANJI_2 136
#define KEY_KANJI_3 137
#define KEY_KANJI_4 138
#define KEY_KANJI_5 139
#define KEY_KANJI_6 140
#define KEY_KANJI_7 141
#define KEY_KANJI_8 142
#define KEY_KANJI_9 143

#define KEY_LANG_1 144
#define KEY_LANG_2 145
#define KEY_LANG_3 146
#define KEY_LANG_4 147
#define KEY_LANG_5 148
#define KEY_LANG_6 149
#define KEY_LANG_7 150
#define KEY_LANG_8 151
#define KEY_LANG_9 152

#define KEY_CONTROL_LEFT   MODIFIER_INDEX + 0
#define KEY_SHIFT_LEFT     MODIFIER_INDEX + 1
#define KEY_ALT_LEFT       MODIFIER_INDEX + 2
#define KEY_SUPER_LEFT     MODIFIER_INDEX + 3
#define KEY_CONTROL_RIGHT  MODIFIER_INDEX + 4
#define KEY_SHIFT_RIGHT    MODIFIER_INDEX + 5
#define KEY_ALT_RIGHT      MODIFIER_INDEX + 6
#define KEY_SUPER_RIGHT    MODIFIER_INDEX + 7

#define MOUSE_1            MOUSE_INDEX + 0
#define MOUSE_2            MOUSE_INDEX + 1
#define MOUSE_3            MOUSE_INDEX + 2
#define MOUSE_4            MOUSE_INDEX + 3
#define MOUSE_5            MOUSE_INDEX + 4
#define MOUSE_SCROLL_UP    MOUSE_INDEX + 5
#define MOUSE_SCROLL_DOWN  MOUSE_INDEX + 6
#define MOUSE_X            MOUSE_INDEX + 7
#define MOUSE_Y            MOUSE_INDEX + 8
#define MOUSE_X_NEG        MOUSE_INDEX + 9
#define MOUSE_Y_NEG        MOUSE_INDEX + 10

// Gamepad buttons sorted as in XInput.
#define GAMEPAD_UP       GAMEPAD_INDEX + 0
#define GAMEPAD_DOWN     GAMEPAD_INDEX + 1
#define GAMEPAD_LEFT     GAMEPAD_INDEX + 2
#define GAMEPAD_RIGHT    GAMEPAD_INDEX + 3
#define GAMEPAD_START    GAMEPAD_INDEX + 4
#define GAMEPAD_SELECT   GAMEPAD_INDEX + 5
#define GAMEPAD_L3       GAMEPAD_INDEX + 6
#define GAMEPAD_R3       GAMEPAD_INDEX + 7
#define GAMEPAD_L1       GAMEPAD_INDEX + 8
#define GAMEPAD_R1       GAMEPAD_INDEX + 9
#define GAMEPAD_HOME     GAMEPAD_INDEX + 10
#define GAMEPAD_A        GAMEPAD_INDEX + 12  // Index 11 is padding.
#define GAMEPAD_B        GAMEPAD_INDEX + 13
#define GAMEPAD_X        GAMEPAD_INDEX + 14
#define GAMEPAD_Y        GAMEPAD_INDEX + 15

#define GAMEPAD_AXIS_LX      GAMEPAD_AXIS_INDEX + 0
#define GAMEPAD_AXIS_LY      GAMEPAD_AXIS_INDEX + 1
#define GAMEPAD_AXIS_LZ      GAMEPAD_AXIS_INDEX + 2
#define GAMEPAD_AXIS_RX      GAMEPAD_AXIS_INDEX + 3
#define GAMEPAD_AXIS_RY      GAMEPAD_AXIS_INDEX + 4
#define GAMEPAD_AXIS_RZ      GAMEPAD_AXIS_INDEX + 5
#define GAMEPAD_AXIS_LX_NEG  GAMEPAD_AXIS_INDEX + 6
#define GAMEPAD_AXIS_LY_NEG  GAMEPAD_AXIS_INDEX + 7
#define GAMEPAD_AXIS_LZ_NEG  GAMEPAD_AXIS_INDEX + 8
#define GAMEPAD_AXIS_RX_NEG  GAMEPAD_AXIS_INDEX + 9
#define GAMEPAD_AXIS_RY_NEG  GAMEPAD_AXIS_INDEX + 10
#define GAMEPAD_AXIS_RZ_NEG  GAMEPAD_AXIS_INDEX + 11

#define PROC_HOME        PROC_INDEX + 0
#define PROC_PROFILE_1   PROC_INDEX + 1
#define PROC_PROFILE_2   PROC_INDEX + 2
#define PROC_PROFILE_3   PROC_INDEX + 3
#define PROC_PROFILE_4   PROC_INDEX + 4
#define PROC_PROFILE_5   PROC_INDEX + 5
#define PROC_PROFILE_6   PROC_INDEX + 6
#define PROC_PROFILE_7   PROC_INDEX + 7
#define PROC_PROFILE_8   PROC_INDEX + 8
#define PROC_PROFILE_9   PROC_INDEX + 9
#define PROC_PROFILE_10  PROC_INDEX + 10
#define PROC_PROFILE_11  PROC_INDEX + 11
#define PROC_PROFILE_12  PROC_INDEX + 12

#define PROC_TUNE_UP          PROC_INDEX + 13
#define PROC_TUNE_DOWN        PROC_INDEX + 14
#define PROC_TUNE_OS          PROC_INDEX + 15
#define PROC_TUNE_MOUSE_SENS  PROC_INDEX + 16
#define PROC_TUNE_TOUCH_SENS  PROC_INDEX + 17
#define PROC_TUNE_DEADZONE    PROC_INDEX + 18
#define PROC_CALIBRATE        PROC_INDEX + 19
#define PROC_RESTART          PROC_INDEX + 20
#define PROC_BOOTSEL          PROC_INDEX + 21
#define PROC_RESET_FACTORY    PROC_INDEX + 22
#define PROC_RESET_CONFIG     PROC_INDEX + 23
#define PROC_RESET_PROFILES   PROC_INDEX + 24
#define PROC_THANKS           PROC_INDEX + 25
#define PROC_HOME_GAMEPAD     PROC_INDEX + 26

#define PROC_MACRO_1          PROC_INDEX + 27
#define PROC_MACRO_2          PROC_INDEX + 28
#define PROC_MACRO_3          PROC_INDEX + 29
#define PROC_MACRO_4          PROC_INDEX + 30
#define PROC_MACRO_5          PROC_INDEX + 31
#define PROC_MACRO_6          PROC_INDEX + 32
#define PROC_MACRO_7          PROC_INDEX + 33
#define PROC_MACRO_8          PROC_INDEX + 34

#define PROC_ROTARY_MODE_0  PROC_INDEX + 35
#define PROC_ROTARY_MODE_1  PROC_INDEX + 36
#define PROC_ROTARY_MODE_2  PROC_INDEX + 37
#define PROC_ROTARY_MODE_3  PROC_INDEX + 38
#define PROC_ROTARY_MODE_4  PROC_INDEX + 39
#define PROC_ROTARY_MODE_5  PROC_INDEX + 40

#define PROC_IGNORE_LED_WARNINGS  PROC_INDEX + 41


void hid_thanks();
void hid_matrix_reset(uint8_t keep);
void hid_press(uint8_t key);
void hid_release(uint8_t key);
void hid_press_multiple(uint8_t *keys);
void hid_release_multiple(uint8_t *keys);
void hid_press_later(uint8_t key, uint16_t delay);
void hid_release_later(uint8_t key, uint16_t delay);
void hid_press_multiple_later(uint8_t *keys, uint16_t delay);
void hid_release_multiple_later(uint8_t *keys, uint16_t delay);
void hid_press_later_callback(alarm_id_t alarm, uint8_t key);
void hid_release_later_callback(alarm_id_t alarm, uint8_t key);
void hid_press_multiple_later_callback(alarm_id_t alarm, uint8_t *keys);
void hid_release_multiple_later_callback(alarm_id_t alarm, uint8_t *keys);
void hid_macro(uint8_t index);
bool hid_is_axis(uint8_t key);
void hid_mouse_move(int16_t x, int16_t y);
void hid_mouse_wheel(int8_t z);
void hid_gamepad_lx(double value);
void hid_gamepad_ly(double value);
void hid_gamepad_rx(double value);
void hid_gamepad_ry(double value);
void hid_gamepad_lz(double value);
void hid_gamepad_rz(double value);
void hid_report();
void hid_init();

extern bool hid_allow_communication;
