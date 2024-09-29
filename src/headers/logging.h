// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#pragma once

typedef enum _LogLevel {
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_DEBUG,
} LogLevel;

typedef enum _LogMask {
    LOG_BASIC = 0,
    LOG_USB = 1,
    LOG_TOUCH_SENS = 2,
    LOG_WIRELESS = 4,
} LogMask;

void logging_set_onloop(bool value);
bool logging_get_onloop();

void logging_set_level(LogLevel level);
LogLevel logging_get_level();

void logging_set_mask(LogMask mask);
bool logging_has_mask(LogMask mask);
LogMask logging_get_mask();

void logging_init();
void logging_load_from_config();
void info(char *msg, ...);
void warn(char *msg, ...);
void error(char *msg, ...);
void debug(char *msg, ...);
void debug_uart(char *msg, ...);
