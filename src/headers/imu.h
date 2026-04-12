// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#pragma once
#include "vector.h"

// LSM6DSR
#define IMU_READ 0b10000000 
#define IMU_WHO_AM_I 0x0f    // Identifier address.
#define IMU_CTRL1_XL 0x10    // Accelerometer config address.
#define IMU_CTRL2_G 0x11     // Gyroscope config address.
#define IMU_CTRL3_C 0x12     // IMU config address.
#define IMU_CTRL4_C 0x13     // IMU config4 address. Has LPF1 enable.
#define IMU_CTRL6_C 0x15     // IMU LPF config address.
#define IMU_CTRL8_XL 0x17    // Accelerometer filter config address.
#define IMU_OUTX_L_G 0x22    // Gyroscope read X address.
#define IMU_OUTY_L_G 0x24    // Gyroscope read Y address.
#define IMU_OUTZ_L_G 0x26    // Gyroscope read Z address.
#define IMU_OUTX_L_XL 0x28   // Accelerometer read X address.
#define IMU_OUTY_L_XL 0x30   // Accelerometer read Y address.
#define IMU_OUTZ_L_XL 0x2A   // Accelerometer read Z address.


#define IMU_CTRL1_XL_OFF 0b00000000  // Accelerometer value power off.
#define IMU_CTRL1_XL_2G  0b10100010  // Accelerometer value for 2G range, LPF2 enabled
#define IMU_CTRL1_XL_2G_NOFILT  0b10100000  // Accelerometer value for 2G range, LPF2 disabled

#define IMU_CTRL3_C_BOOT 0b00000001  // Reboot the IMU.
#define IMU_CTRL3_C_BDU  0b01000000  // Block data update, prevents reading inconsistent data.

/* XL = 100b that is ODR/100 bandwidth, 66Hz for 6.6kHz ODR */
#define IMU_CTRL8_XL_LP  0b10000000  // Accelerometer value for low pass filter.
#define IMU_CTRL2_G_OFF  0b00000000  // Gyroscope value power off.
#define IMU_CTRL2_G_125  0b10100010  // Gyroscope value for 125 dps.
#define IMU_CTRL2_G_250  0b10100011  // Gyroscope value for 125 dps.
#define IMU_CTRL2_G_500  0b10100100  // Gyroscope value for 500 dps.
#define IMU_CTRL2_G_1000 0b10100101  // Gyroscope value for 1000 dps.

#define GYRO_LPF1_ENABLE_CTRL4_C  0b00000010  // enable LPF1 in CTRL4_C register
#define GYRO_LPF1_DISABLE_CTRL4_C 0b00000000 // disable LPF1 in CTRL4_C register
#define GYRO_LPF1_470HZ_CTRL6_C   0b00000011   // FTYPE selection for LPF1, 470Hz cutoff at 6.6kHz ODR
#define GYRO_LPF1_223HZ_CTRL6_C   0b00000001   // FTYPE selection for LPF1, 223Hz cutoff at 6.6kHz ODR
#define GYRO_LPF1_297HZ_CTRL6_C   0b00000000   // FTYPE selection for LPF1, 297Hz cutoff at 6.6kHz ODR

#define GYRO_FS_REG_125  0b00000010  // Gyro full scale register for 125 dps.
#define GYRO_FS_REG_250  0b00000000  // Gyro full scale register for 250 dps.
#define GYRO_FS_REG_500  0b00000100  // Gyro full scale register for 500 dps.
#define GYRO_FS_REG_1000 0b00001000  // Gyro full scale register for 1000 dps.
#define GYRO_FS_REG_2000 0b00001100  // Gyro full scale register for 2000 dps.
#define GYRO_FS_REG_4000 0b00000001  // Gyro full scale register for 4000 dps.

/* Gyro output data rate options (Hz)*/
#define GYRO_ODR_REG_12_5 0b00010000
#define GYRO_ODR_REG_26   0b00100000
#define GYRO_ODR_REG_52   0b00110000
#define GYRO_ODR_REG_104  0b01000000
#define GYRO_ODR_REG_208  0b01010000
#define GYRO_ODR_REG_416  0b01100000
#define GYRO_ODR_REG_833  0b01110000
#define GYRO_ODR_REG_1660 0b10000000
#define GYRO_ODR_REG_3330 0b10010000
#define GYRO_ODR_REG_6660 0b10100000

/* Gyro int-to-float conversion constants (rad/s)*/
#define GYRO_SENS_RADPS_125  7.63581547747519E-05f
#define GYRO_SENS_RADPS_250  0.000152716309549504f
#define GYRO_SENS_RADPS_500  0.000305432619099008f
#define GYRO_SENS_RADPS_1000 0.000610865238198015f
#define GYRO_SENS_RADPS_2000 0.00122173047639603f
#define GYRO_SENS_RADPS_4000 0.00244346095279206f

#define GYRO_SENS_DEGPS_125  4.375E-03f
#define GYRO_SENS_DEGPS_250  8.75E-03f
#define GYRO_SENS_DEGPS_500  1.75E-2f
#define GYRO_SENS_DEGPS_1000 3.5E-2f
#define GYRO_SENS_DEGPS_2000 7E-2f
#define GYRO_SENS_DEGPS_4000 14E-2f

#define RAD_2_DEG 57.2957795130823f
#define DEG_2_RAD 0.0174532925199433f
#define ACCEL_G 9.80665f  // Standard gravity in m/s^2.



#define ACCEL_SENS_2G 0.00059841f
#define ACCEL_SENS_4G 0.00119682f
#define ACCEL_SENS_8G 0.00239364f
#define ACCEL_SENS_16G 0.00478728f

// Highest value in integer, needed to reach the respective dps range value.
// E.g. GYRO_SENS_DEGPS_500 * 28571 ~= 500
#define GYRO_MAXVAL_INT 28571



#define GYRO_USER_OFFSET_FACTOR 1.5

void imu_init();
void imu_power_off();
Vector imu_read_gyro();
Vector imu_read_accel();
void imu_load_calibration();
void imu_calibrate();

typedef struct calib_data_struct {
    float offset;
    float variance;
    float stddev;
} CalibData;

typedef struct imu_calib_struct {
    CalibData gyro[3];
    CalibData accel[3];
} ImuCalib;