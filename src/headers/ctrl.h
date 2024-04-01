// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#pragma once
#include "common.h"

#define CTRL_PROTOCOL_VERSION 1
#define CTRL_MSG_SIZE 64
#define CTRL_NON_PAYLOAD_SIZE 4
#define CTRL_MAX_PAYLOAD_SIZE (CTRL_MSG_SIZE - CTRL_NON_PAYLOAD_SIZE)

typedef enum Ctrl_device_enum {
    ALPAKKA = 1,
    KAPYBARA,
} Ctrl_device;

typedef enum Ctrl_msg_type_enum {
    LOG = 1,
    PROC,
    CONFIG_GET,
    CONFIG_SET,
    CONFIG_SHARE,
    PROFILE_GET,
    PROFILE_SET,
    PROFILE_SHARE,
    ESTABLISH_DEVICE,
    ESTABLISH_HOST,
} Ctrl_msg_type;

typedef enum Ctrl_cfg_type_enum {
    PROTOCOL = 1,
    SENS_TOUCH,
    SENS_MOUSE,
    DEADZONE,
} Ctrl_cfg_type;

typedef enum CtrlSectionType_enum {
    SECTION_NAME = 1,
    SECTION_A,
    SECTION_B,
    SECTION_X,
    SECTION_Y,
    SECTION_DPAD_LEFT,
    SECTION_DPAD_RIGHT,
    SECTION_DPAD_UP,
    SECTION_DPAD_DOWN,
    SECTION_SELECT_1,
    SECTION_START_1,
    SECTION_SELECT_2,
    SECTION_START_2,
    SECTION_L1,
    SECTION_R1,
    SECTION_L2,
    SECTION_R2,
    SECTION_L4,
    SECTION_R4,
    SECTION_DHAT_LEFT,
    SECTION_DHAT_RIGHT,
    SECTION_DHAT_UP,
    SECTION_DHAT_DOWN,
    SECTION_DHAT_UL,
    SECTION_DHAT_UR,
    SECTION_DHAT_DL,
    SECTION_DHAT_DR,
    SECTION_DHAT_PUSH,
    SECTION_ROTARY_UP,
    SECTION_ROTARY_DOWN,
    SECTION_THUMBSTICK,
    SECTION_THUMBSTICK_LEFT,
    SECTION_THUMBSTICK_RIGHT,
    SECTION_THUMBSTICK_UP,
    SECTION_THUMBSTICK_DOWN,
    SECTION_THUMBSTICK_PUSH,
    SECTION_THUMBSTICK_INNER,
    SECTION_THUMBSTICK_OUTER,
    SECTION_GLYPHS_0,
    SECTION_GLYPHS_1,
    SECTION_GLYPHS_2,
    SECTION_GLYPHS_3,
    SECTION_DAISY_0,
    SECTION_DAISY_1,
    SECTION_DAISY_2,
    SECTION_DAISY_3,
    SECTION_GYRO,
    SECTION_GYRO_X,
    SECTION_GYRO_Y,
    SECTION_GYRO_Z,
    SECTION_MACRO_1,
    SECTION_MACRO_2,
    SECTION_MACRO_3,
    SECTION_MACRO_4,
} CtrlSectionType;

typedef struct Ctrl_struct {
    uint8_t protocol_version;
    uint8_t device_id;
    Ctrl_msg_type message_type;
    uint8_t len;
    uint8_t payload[CTRL_MAX_PAYLOAD_SIZE];
} Ctrl;

typedef struct CtrlProfileName_struct {
    // Must be packed (58 bytes).
    char name[24];
    char padding[34];
} CtrlProfileName;

typedef struct CtrlButton_struct {
    // Must be packed (58 bytes).
    uint8_t mode;
    uint8_t reserved;
    uint8_t actions[4];
    uint8_t actions_secondary[4];
    uint8_t actions_reserved[16];
    uint8_t chords[4];
    uint8_t hint[14];
    uint8_t hint_secondary[14];
} CtrlButton;

typedef struct CtrlRotary_struct {
    // Must be packed (58 bytes).
    uint8_t actions_0[4];
    uint8_t actions_1[4];
    uint8_t actions_2[4];
    uint8_t actions_3[4];
    uint8_t actions_4[4];
    uint8_t hint_0[14];
    uint8_t hint_1[6];
    uint8_t hint_2[6];
    uint8_t hint_3[6];
    uint8_t hint_4[6];
} CtrlRotary;

typedef struct CtrlThumbstick_struct {
    // Must be packed (58 bytes).
    uint8_t mode;
    uint8_t distance_mode;
    uint8_t deadzone;
    uint8_t overlap;
    uint8_t deadzone_override;
    uint8_t padding[53];
} CtrlThumbstick;

typedef struct CtrlGlyph_struct {
    uint8_t actions[4];
    uint8_t glyph;
} CtrlGlyph;

typedef struct CtrlGlyphs_struct {
    // Must be packed (58 bytes).
    CtrlGlyph glyphs[11];
    uint8_t padding[3];
} CtrlGlyphs;

typedef struct CtrlDaisyGroup_struct {
    uint8_t actions_a[4];
    uint8_t actions_b[4];
    uint8_t actions_x[4];
    uint8_t actions_y[4];
} CtrlDaisyGroup;

typedef struct CtrlDaisy_struct {
    // Must be packed (58 bytes).
    CtrlDaisyGroup groups[2];
    uint8_t padding[26];
} CtrlDaisy;

typedef struct CtrlGyro_struct {
    // Must be packed (58 bytes).
    uint8_t mode;
    uint8_t engage;
    uint8_t padding[56];
} CtrlGyro;

typedef struct CtrlGyroAxis_struct {
    // Must be packed (58 bytes).
    uint8_t actions_neg[4];
    uint8_t actions_pos[4];
    uint8_t angle_min;
    uint8_t angle_max;
    uint8_t hint_neg[14];
    uint8_t hint_pos[14];
    uint8_t padding[20];
} CtrlGyroAxis;

typedef struct CtrlMacro_struct {
    // Must be packed (58 bytes).
    uint8_t macro[2][28];
    uint8_t padding[2];
} CtrlMacro;

typedef union CtrlSection_union {
    CtrlProfileName name;
    CtrlButton button;
    CtrlRotary rotary;
    CtrlThumbstick thumbstick;
    CtrlGlyphs glyphs;
    CtrlDaisy daisy;
    CtrlGyro gyro;
    CtrlGyroAxis gyro_axis;
    CtrlMacro macro;
} CtrlSection;

typedef struct CtrlProfile_struct {
    CtrlSection sections[64];
} CtrlProfile;

Ctrl ctrl_establish_device();
Ctrl ctrl_log(uint8_t* offset_ptr, uint8_t len);
Ctrl ctrl_config_share(uint8_t index);
Ctrl ctrl_profile_share(uint8_t profile_index, uint8_t section_index);
