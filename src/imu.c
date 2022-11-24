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
double offset_1_x;
double offset_1_y;

void imu_init_single(uint8_t cs, uint8_t gyro_conf) {
    bus_spi_write(cs, IMU_CTRL2_G, gyro_conf);
    uint8_t ctrl = bus_spi_read_one(cs, IMU_CTRL2_G);
    printf("  IMU cs=%i ctrl2_g=0x%i\n", cs, bin(ctrl));
}

void imu_init() {
    printf("Config IMU\n");
    imu_init_single(PIN_SPI_CS0, IMU_CTRL2_G_500);
    imu_init_single(PIN_SPI_CS1, IMU_CTRL2_G_125);
    config_nvm_t config;
    config_read(&config);
    offset_0_x = config.imu_0_offset_x;
    offset_0_y = config.imu_0_offset_y;
    offset_1_x = config.imu_1_offset_x;
    offset_1_y = config.imu_1_offset_y;
    imu_update_sensitivity();
}

vector_t imu_read_gyro(uint8_t cs) {
    uint8_t buf[6];
    bus_spi_read(cs, IMU_OUTX_L_G, buf, 6);
    int16_t y =  (((int8_t)buf[1] << 8) + (int8_t)buf[0]);
    int16_t z =  (((int8_t)buf[3] << 8) + (int8_t)buf[2]);
    int16_t x = -(((int8_t)buf[5] << 8) + (int8_t)buf[4]);
    double offset_x = (cs==PIN_SPI_CS0) ? offset_0_x : offset_1_x;
    double offset_y = (cs==PIN_SPI_CS0) ? offset_0_y : offset_1_y;
    return (vector_t){
        (double)x - offset_x,
        (double)y - offset_y,
    };
}

vector_t imu_read_gyro_burst(uint8_t cs, uint8_t samples) {
    double x = 0;
    double y = 0;
    for(uint8_t i=0; i<samples; i++) {
        vector_t sample = imu_read_gyro(cs);
        x += sample.x;
        y += sample.y;
    }
    x /= samples;
    y /= samples;
    return (vector_t){x, y};
}

vector_t imu_read_gyros() {
    vector_t imu0 = imu_read_gyro_burst(PIN_SPI_CS0, CFG_IMU_TICK_SAMPLES/8*1);
    vector_t imu1 = imu_read_gyro_burst(PIN_SPI_CS1, CFG_IMU_TICK_SAMPLES/8*7);
    double weight = max(abs(imu1.x), abs(imu1.y)) / 32768.0;
    double weight_0 = ramp_mid(weight, 0.2);
    double weight_1 = 1 - weight_0;
    double x = (imu0.x * weight_0) + (imu1.x * weight_1 / 4);
    double y = (imu0.y * weight_0) + (imu1.y * weight_1 / 4);
    return (vector_t){x, y};
}

double hssnf(double t, double k, double x) {
    double a = x - (x * k);
    double b = 1 - (x * k * (1/t));
    return a / b;
}

void imu_report() {
    if (touch_status() == false) return;
    static double mouse_sub_x = 0;
    static double mouse_sub_y = 0;
    // Read gyro and apply sensitivity.
    vector_t gyro = imu_read_gyros();
    double mouse_x = gyro.x * CFG_MOUSE_SENSITIVITY_X * sensitivity_multiplier;
    double mouse_y = gyro.y * CFG_MOUSE_SENSITIVITY_Y * sensitivity_multiplier;
    // Magic happens.
    double t = 1.0;
    double k = 0.5;
    if      (mouse_x > 0 && mouse_x <  t) mouse_x =  hssnf(t, k,  mouse_x);
    else if (mouse_x < 0 && mouse_x > -t) mouse_x = -hssnf(t, k, -mouse_x);
    if      (mouse_y > 0 && mouse_y <  t) mouse_y =  hssnf(t, k,  mouse_y);
    else if (mouse_y < 0 && mouse_y > -t) mouse_y = -hssnf(t, k, -mouse_y);
    // Reintroduce subpixel leftovers.
    mouse_x += mouse_sub_x;
    mouse_y += mouse_sub_y;
    // Round down and save leftovers.
    mouse_sub_x = modf(mouse_x, &mouse_x);
    mouse_sub_y = modf(mouse_y, &mouse_y);
    // Report.
    hid_mouse_move((int16_t)mouse_x, (int16_t)mouse_y);
}

void imu_calibrate_single(uint8_t cs) {
    printf("IMU: cs=%i calibrating...\n", cs);
    uint32_t len = 200000;
    if (cs == PIN_SPI_CS0) {
        offset_0_x = 0;
        offset_0_y = 0;
    }
    if (cs == PIN_SPI_CS1) {
        offset_1_x = 0;
        offset_1_y = 0;
    }
    double x = 0;
    double y = 0;
    uint32_t i = 0;
    while(i < len) {
        if (!(i % 5000)) led_cycle_step();
        vector_t sample = imu_read_gyro(cs);
        x += sample.x;
        y += sample.y;
        i++;
    }
    x /= len;
    y /= len;
    printf("IMU: cs=%i calibration x=%f y=%f\n", cs, x, y);
    if (cs == PIN_SPI_CS0) {
        offset_0_x = x;
        offset_0_y = y;
    }
    if (cs == PIN_SPI_CS1) {
        offset_1_x = x;
        offset_1_y = y;
    }

}

void imu_calibrate() {
    imu_calibrate_single(PIN_SPI_CS0);
    imu_calibrate_single(PIN_SPI_CS1);
    config_set_imu_offset(offset_0_x, offset_0_y, offset_1_x, offset_1_y);
}

void imu_update_sensitivity() {
    config_nvm_t config;
    config_read(&config);
    float multipliers[3] = {
        CFG_MOUSE_SENSITIVITY_MULTIPLIER_LOW,
        CFG_MOUSE_SENSITIVITY_MULTIPLIER_MID,
        CFG_MOUSE_SENSITIVITY_MULTIPLIER_HIGH
    };
    sensitivity_multiplier = multipliers[config.sensitivity];
}
