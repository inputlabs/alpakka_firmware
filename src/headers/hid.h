// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#pragma once
#include <pico/time.h>

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
#define KEY_LEFT_BRACKET 47
#define KEY_RIGHT_BRACKET 48
#define KEY_BACKSLASH 49
#define KEY_SEMICOLON 51
#define KEY_QUOTE 52
#define KEY_BACKQUOTE 53
#define KEY_COMMA 54
#define KEY_PERIOD 55
#define KEY_SLASH 56
#define KEY_CAPS_LOCK 57

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
#define KEY_PAD_BACKSLASH 100
#define KEY_PAD_EQUAL 103

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

#define MODIFIER_INDEX 120
#define MOUSE_INDEX 140
#define GAMEPAD_INDEX 160
#define GAMEPAD_AXIS_INDEX 180
#define PROC_INDEX 200

#define KEY_LEFT_CONTROL   MODIFIER_INDEX + 0
#define KEY_LEFT_SHIFT     MODIFIER_INDEX + 1
#define KEY_LEFT_ALT       MODIFIER_INDEX + 2
#define KEY_LEFT_SUPER     MODIFIER_INDEX + 3
#define KEY_RIGHT_CONTROL  MODIFIER_INDEX + 4
#define KEY_RIGHT_SHIFT    MODIFIER_INDEX + 5
#define KEY_RIGHT_ALT      MODIFIER_INDEX + 6
#define KEY_RIGHT_SUPER    MODIFIER_INDEX + 7

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
#define GAMEPAD_A        GAMEPAD_INDEX + 12
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
#define GAMEPAD_AXIS_RX_NEG  GAMEPAD_AXIS_INDEX + 8
#define GAMEPAD_AXIS_RY_NEG  GAMEPAD_AXIS_INDEX + 9

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

#define PROC_TUNE_UP               PROC_INDEX + 13
#define PROC_TUNE_DOWN             PROC_INDEX + 14
#define PROC_TUNE_OS               PROC_INDEX + 15
#define PROC_TUNE_SENSITIVITY      PROC_INDEX + 16
#define PROC_TUNE_DEADZONE         PROC_INDEX + 17
#define PROC_TUNE_TOUCH_THRESHOLD  PROC_INDEX + 18
#define PROC_TUNE_VIBRATION        PROC_INDEX + 19
#define PROC_CALIBRATE             PROC_INDEX + 20
#define PROC_BOOTSEL               PROC_INDEX + 21
#define PROC_THANKS                PROC_INDEX + 22
#define PROC_MACRO                 PROC_INDEX + 23
#define PROC_HOME_GAMEPAD          PROC_INDEX + 24

#define PROC_ADZ    PROC_INDEX + 25
#define PROC_ADZN   PROC_INDEX + 26

#define PROC_ROTARY_MODE_0  PROC_INDEX + 27
#define PROC_ROTARY_MODE_1  PROC_INDEX + 28
#define PROC_ROTARY_MODE_2  PROC_INDEX + 29
#define PROC_ROTARY_MODE_3  PROC_INDEX + 30
#define PROC_ROTARY_MODE_4  PROC_INDEX + 31

void hid_thanks();
void hid_matrix_reset();
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
bool hid_is_axis(uint8_t key);
void hid_mouse_move(int16_t x, int16_t y);
void hid_mouse_wheel(int8_t z);
void hid_gamepad_lx(int16_t value);
void hid_gamepad_ly(int16_t value);
void hid_gamepad_rx(int16_t value);
void hid_gamepad_ry(int16_t value);
void hid_gamepad_lz(uint16_t value);
void hid_gamepad_rz(uint16_t value);
void hid_report();
void hid_init();

extern bool hid_allow_communication;
