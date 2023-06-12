// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pico/stdlib.h>
#include <hardware/gpio.h>
#include <hardware/i2c.h>
#include <hardware/spi.h>
#include "imu.h"
#include "config.h"
#include "helper.h"
#include "pin.h"
#include "bus.h"
#include "touch.h"
#include "hid.h"
#include "led.h"
#include "helper.h"

double sensitivity_multiplier;
double offset_0_x;
double offset_0_y;
double offset_0_z;
double offset_1_x;
double offset_1_y;
double offset_1_z;

void imu_init_single(uint8_t cs, uint8_t gyro_conf) {
    bus_spi_write(cs, IMU_CTRL1_XL, 0b10100010);
    bus_spi_write(cs, IMU_CTRL8_XL, 0b00000000);
    bus_spi_write(cs, IMU_CTRL2_G, gyro_conf);
    uint8_t ctrl1 = bus_spi_read_one(cs, IMU_CTRL1_XL);
    uint8_t ctrl2 = bus_spi_read_one(cs, IMU_CTRL2_G);
    printf("  IMU cs=%i xl=0x%08i g=0x%08i\n", cs, bin(ctrl1), bin(ctrl2));
}

void imu_init() {
    printf("INIT: IMU\n");
    imu_init_single(PIN_SPI_CS0, IMU_CTRL2_G_500);
    imu_init_single(PIN_SPI_CS1, IMU_CTRL2_G_125);
    config_nvm_t config;
    config_read(&config);
    offset_0_x = config.imu_0_offset_x;
    offset_0_y = config.imu_0_offset_y;
    offset_0_z = config.imu_0_offset_z;
    offset_1_x = config.imu_1_offset_x;
    offset_1_y = config.imu_1_offset_y;
    offset_1_z = config.imu_1_offset_z;
    imu_update_sensitivity();
}

vector_t imu_read_gyro_bits(uint8_t cs) {
    uint8_t buf[6];
    bus_spi_read(cs, IMU_OUTX_L_G, buf, 6);
    int16_t y =  (((int8_t)buf[1] << 8) + (int8_t)buf[0]);
    int16_t z =  (((int8_t)buf[3] << 8) + (int8_t)buf[2]);
    int16_t x = -(((int8_t)buf[5] << 8) + (int8_t)buf[4]);
    double offset_x = (cs==PIN_SPI_CS0) ? offset_0_x : offset_1_x;
    double offset_y = (cs==PIN_SPI_CS0) ? offset_0_y : offset_1_y;
    double offset_z = (cs==PIN_SPI_CS0) ? offset_0_z : offset_1_z;
    return (vector_t){
        (double)x - offset_x,
        (double)y - offset_y,
        (double)z - offset_z,
    };
}

vector_t imu_read_accel_bits(uint8_t cs) {
    uint8_t buf[6];
    bus_spi_read(cs, IMU_OUTX_L_XL, buf, 6);
    int16_t x =  (((int8_t)buf[1] << 8) + (int8_t)buf[0]);
    int16_t y =  (((int8_t)buf[3] << 8) + (int8_t)buf[2]);
    int16_t z =  (((int8_t)buf[5] << 8) + (int8_t)buf[4]);
    // double offset_x = (cs==PIN_SPI_CS0) ? offset_0_x : offset_1_x;
    // double offset_y = (cs==PIN_SPI_CS0) ? offset_0_y : offset_1_y;
    // double offset_z = (cs==PIN_SPI_CS0) ? offset_0_z : offset_1_z;
    return (vector_t){
        limit_between(-1, (double)x / 16384.0, 1),
        limit_between(-1, (double)y / 16384.0, 1),
        limit_between(-1, (double)z / 16384.0, 1),
    };
}

vector_t imu_read_gyro_burst(uint8_t cs, uint8_t samples) {
    double x = 0;
    double y = 0;
    double z = 0;
    for(uint8_t i=0; i<samples; i++) {
        vector_t sample = imu_read_gyro_bits(cs);
        x += sample.x;
        y += sample.y;
        z += sample.z;
    }
    x /= samples;
    y /= samples;
    z /= samples;
    return (vector_t){x, y, z};
}

vector_t imu_read_gyros() {
    vector_t imu0 = imu_read_gyro_burst(PIN_SPI_CS0, CFG_IMU_TICK_SAMPLES/8*1);
    vector_t imu1 = imu_read_gyro_burst(PIN_SPI_CS1, CFG_IMU_TICK_SAMPLES/8*7);
    double weight = max(abs(imu1.x), abs(imu1.y)) / 32768.0;
    double weight_0 = ramp_mid(weight, 0.2);
    double weight_1 = 1 - weight_0;
    double x = (imu0.x * weight_0) + (imu1.x * weight_1 / 4);
    double y = (imu0.y * weight_0) + (imu1.y * weight_1 / 4);
    double z = (imu0.z * weight_0) + (imu1.z * weight_1 / 4);
    return (vector_t){x, y, z};
}

double hssnf(double t, double k, double x) {
    double a = x - (x * k);
    double b = 1 - (x * k * (1/t));
    return a / b;
}

vector_t imu_read_gyro() {
    static double sub_x = 0;
    static double sub_y = 0;
    static double sub_z = 0;
    // Read gyro values.
    vector_t gyro = imu_read_gyros();
    double x = gyro.x * CFG_GYRO_SENSITIVITY_X * sensitivity_multiplier;
    double y = gyro.y * CFG_GYRO_SENSITIVITY_Y * sensitivity_multiplier;
    double z = gyro.z * CFG_GYRO_SENSITIVITY_Z * sensitivity_multiplier;
    // Magic happens.
    double t = 1.0;
    double k = 0.5;
    if      (x > 0 && x <  t) x =  hssnf(t, k,  x);
    else if (x < 0 && x > -t) x = -hssnf(t, k, -x);
    if      (y > 0 && y <  t) y =  hssnf(t, k,  y);
    else if (y < 0 && y > -t) y = -hssnf(t, k, -y);
    if      (z > 0 && z <  t) z =  hssnf(t, k,  z);
    else if (z < 0 && z > -t) z = -hssnf(t, k, -z);
    // Reintroduce subpixel leftovers.
    x += sub_x;
    y += sub_y;
    z += sub_z;
    // Round down and save leftovers.
    sub_x = modf(x, &x);
    sub_y = modf(y, &y);
    sub_z = modf(z, &z);
    // Return 3d vector.
    return (vector_t){x, y, z};
}

vector_t imu_read_gyro_alt() {
    // Read gyro values.
    vector_t gyro = imu_read_gyros();
    double x = gyro.x * CFG_GYRO_SENSITIVITY_X;
    double y = gyro.y * CFG_GYRO_SENSITIVITY_Y;
    double z = gyro.z * CFG_GYRO_SENSITIVITY_Z;
    // Return 3d vector.
    return (vector_t){x, y, z};
}

vector_t imu_read_accel() {
    vector_t accel0 = imu_read_accel_bits(PIN_SPI_CS0);
    vector_t accel1 = imu_read_accel_bits(PIN_SPI_CS1);
    return (vector_t){
        (accel0.x + accel1.x) / 2,
        (accel0.y + accel1.y) / 2,
        (accel0.z + accel1.z) / 2
    };
}

void imu_calibrate_single(uint8_t cs) {
    printf("IMU: cs=%i calibrating...\n", cs);
    uint32_t len = 200000;
    if (cs == PIN_SPI_CS0) {
        offset_0_x = 0;
        offset_0_y = 0;
        offset_0_z = 0;
    }
    if (cs == PIN_SPI_CS1) {
        offset_1_x = 0;
        offset_1_y = 0;
        offset_1_z = 0;
    }
    double x = 0;
    double y = 0;
    double z = 0;
    uint32_t i = 0;
    while(i < len) {
        if (!(i % 5000)) led_cycle_step();
        vector_t sample = imu_read_gyro_bits(cs);
        x += sample.x;
        y += sample.y;
        z += sample.z;
        i++;
    }
    x /= len;
    y /= len;
    z /= len;
    printf("IMU: cs=%i calibration x=%f y=%f z=%f\n", cs, x, y, z);
    if (cs == PIN_SPI_CS0) {
        offset_0_x = x;
        offset_0_y = y;
        offset_0_z = z;
    }
    if (cs == PIN_SPI_CS1) {
        offset_1_x = x;
        offset_1_y = y;
        offset_1_z = z;
    }
}

void imu_calibrate() {
    imu_calibrate_single(PIN_SPI_CS0);
    imu_calibrate_single(PIN_SPI_CS1);
    config_set_imu_offset(
        offset_0_x,
        offset_0_y,
        offset_0_z,
        offset_1_x,
        offset_1_y,
        offset_1_z
    );
}

void imu_update_sensitivity() {
    config_nvm_t config;
    config_read(&config);
    float multipliers[3] = {
        CFG_GYRO_SENSITIVITY_MULTIPLIER_LOW,
        CFG_GYRO_SENSITIVITY_MULTIPLIER_MID,
        CFG_GYRO_SENSITIVITY_MULTIPLIER_HIGH
    };
    sensitivity_multiplier = multipliers[config.sensitivity];
}
