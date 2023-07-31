// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#pragma once
#include "vector.h"

// LSM6DSR
#define IMU_CTRL1_XL 0x10
#define IMU_CTRL2_G 0x11
#define IMU_CTRL3_C 0x12
#define IMU_CTRL8_XL 0x17

#define IMU_OUTX_L_G 0x22
#define IMU_OUTY_L_G 0x24
#define IMU_OUTZ_L_G 0x26
#define IMU_OUTX_L_XL 0x28
#define IMU_OUTY_L_XL 0x30
#define IMU_OUTZ_L_XL 0x2A

#define IMU_CTRL2_G_125 0b10100010
#define IMU_CTRL2_G_500 0b10100100

void imu_init();
Vector imu_read_gyro();
Vector imu_read_accel();
void imu_calibrate();

