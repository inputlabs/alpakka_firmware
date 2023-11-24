// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#pragma once
#include <stdint.h>
#include "common.h"

#define WEBUSB_BUFFER_SIZE 2048
#define CTRL_VERSION 1
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
} CtrlSectionType;

typedef struct Ctrl_struct {
    u8 protocol_version;
    u8 device_id;
    Ctrl_msg_type message_type;
    u8 len;
    u8 payload[CTRL_MAX_PAYLOAD_SIZE];
} Ctrl;

typedef struct CtrlProfileName_struct {
    // Must be packed (58 bytes).
    char name[24];
    char padding[34];
} CtrlProfileName;

typedef struct CtrlButton_struct {
    // Must be packed (58 bytes).
    u8 mode;
    u8 reserved;
    u8 actions[4];
    u8 actions_secondary[4];
    u8 actions_reserved[16];
    u8 chords[4];
    u8 hint[14];
    u8 hint_secondary[14];
} CtrlButton;

typedef struct CtrlRotary_struct {
    // Must be packed (58 bytes).
    u8 actions_0[4];
    u8 actions_1[4];
    u8 actions_2[4];
    u8 actions_3[4];
    u8 actions_4[4];
    u8 hint_0[14];
    u8 hint_1[6];
    u8 hint_2[6];
    u8 hint_3[6];
    u8 hint_4[6];
} CtrlRotary;

typedef struct CtrlThumbstick_struct {
    // Must be packed (58 bytes).
    u8 mode;
    u8 deadzone;
    u8 overlap;
    u8 padding[55];
} CtrlThumbstick;

typedef union CtrlSection_union {
    CtrlProfileName name;
    CtrlButton button;
    CtrlRotary rotary;
    CtrlThumbstick thumbstick;
} CtrlSection;

typedef struct CtrlProfile_struct {
    CtrlSection sections[64];
} CtrlProfile;

void webusb_read();
void webusb_write(char *msg);
bool webusb_flush();
void webusb_flush_force();
void webusb_set_pending_config_share(bool value);
