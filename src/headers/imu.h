// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#pragma once

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

typedef struct vector {
    double x;
    double y;
    double z;
} vector_t;

void imu_init();
vector_t imu_read_gyro();
vector_t imu_read_gyro_alt();
vector_t imu_read_accel();
void imu_calibrate();
void imu_update_sensitivity();

