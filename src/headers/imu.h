// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#pragma once
#include "vector.h"

// LSM6DSR
#define IMU_WHO_AM_I 0x0f  // Identifier address.
#define IMU_CTRL1_XL 0x10  // Accelerometer config address.
#define IMU_CTRL1_XL_2G 0b10100010  // Accelerometer config value for 2G range.
#define IMU_CTRL2_G 0x11  // Gyroscope config address.
#define IMU_CTRL2_G_125 0b10100010  // Gyroscope config value for 125 dps.
#define IMU_CTRL2_G_500 0b10100100  // Gyroscope config value for 500 dps.
#define IMU_CTRL3_C 0x12  // IMU config address.
#define IMU_CTRL8_XL 0x17  // Accelerometer filter config address.
#define IMU_CTRL8_XL_LP 0b00000000  // Accelerometer filter config value.
#define IMU_OUTX_L_G 0x22  // Gyroscope read X address.
#define IMU_OUTY_L_G 0x24  // Gyroscope read Y address.
#define IMU_OUTZ_L_G 0x26  // Gyroscope read Z address.
#define IMU_OUTX_L_XL 0x28  // Accelerometer read X address.
#define IMU_OUTY_L_XL 0x30  // Accelerometer read Y address.
#define IMU_OUTZ_L_XL 0x2A  // Accelerometer read Z address.

void imu_init();
Vector imu_read_gyro();
Vector imu_read_accel();
void imu_load_calibration();
void imu_calibrate();

