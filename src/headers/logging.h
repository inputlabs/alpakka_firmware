// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#pragma once

typedef enum LogLevel_enum {
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_DEBUG,
} LogLevel;

void logging_set_onloop(bool value);
bool logging_get_onloop();

void logging_set_level(LogLevel level);
bool logging_get_level();

void logging_init();
void info(char *msg, ...);
void warn(char *msg, ...);
void error(char *msg, ...);
void debug(char *msg, ...);
void debug_uart(char *msg, ...);
