// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pico/stdlib.h>
#include "imu.h"
#include "config.h"
#include "common.h"
#include "pin.h"
#include "bus.h"
#include "vector.h"
#include "logging.h"

uint8_t IMU0 = 0;
uint8_t IMU1 = 0;
float offset_gyro_0_x;
float offset_gyro_0_y;
float offset_gyro_0_z;
float offset_gyro_1_x;
float offset_gyro_1_y;
float offset_gyro_1_z;
float offset_accel_0_x;
float offset_accel_0_y;
float offset_accel_0_z;
float offset_accel_1_x;
float offset_accel_1_y;
float offset_accel_1_z;

float imu_gyro0_weight[3] = {0.5f, 0.5f, 0.5f};
float imu_gyro1_weight[3] = {0.5f, 0.5f, 0.5f};

ImuCalib imu_calib0 = {
    .gyro = {
        {0, 0, 0}, // Gyro offset  
        {1, 1, 1}, // Gyro variance
        {1, 1, 1}  // Gyro stddev  
    },
    .accel = {
        {0, 0, 0}, // Accel offset  
        {1, 1, 1}, // Accel variance
        {1, 1, 1}  // Accel stddev  
    }
};

ImuCalib imu_calib1 = {
    .gyro = {
        {0, 0, 0}, // Gyro offset  
        {1, 1, 1}, // Gyro variance
        {1, 1, 1}  // Gyro stddev  
    },
    .accel = {
        {0, 0, 0}, // Accel offset
        {1, 1, 1}, // Accel variance
        {1, 1, 1}  // Accel stddev
    }
};

void imu_channel_select() {
    Config *config = config_read();
    IMU0 = config->swap_gyros ? PIN_SPI_CS1 : PIN_SPI_CS0;
    IMU1 = config->swap_gyros ? PIN_SPI_CS0 : PIN_SPI_CS1;
}

void imu_init_single(uint8_t cs, uint8_t gyro_conf) {
    // make sure Accel is ON
    bus_spi_write(cs, IMU_CTRL1_XL, IMU_CTRL1_XL_2G);
    // sleep_ms(10);
    // Reboot the IMU. Accel must be ON before rebooting.
    bus_spi_write(cs, IMU_CTRL3_C, IMU_CTRL3_C_BOOT);
    // Datasheet lists typical turn on time as 35ms, wait 2x that much.
    sleep_ms(120);
    // Read WHO_AM_I register to check if the IMU is present.
    uint8_t id = bus_spi_read_one(cs, IMU_READ | IMU_WHO_AM_I);
    // Accel options.
    bus_spi_write(cs, IMU_CTRL1_XL, IMU_CTRL1_XL_2G);
    bus_spi_write(cs, IMU_CTRL8_XL, IMU_CTRL8_XL_LP);
    // Gyro options.
    // bus_spi_write(cs, IMU_CTRL3_C, IMU_CTRL3_C_BDU); // Block data update
    bus_spi_write(cs, IMU_CTRL4_C, GYRO_LPF1_ENABLE_CTRL4_C); // Enable LPF1
    bus_spi_write(cs, IMU_CTRL6_C, GYRO_LPF1_470HZ_CTRL6_C); // FTYPE selection for LPF1
    bus_spi_write(cs, IMU_CTRL2_G, gyro_conf);
    uint8_t xl = bus_spi_read_one(cs, IMU_READ | IMU_CTRL1_XL);
    uint8_t g = bus_spi_read_one(cs, IMU_READ | IMU_CTRL2_G);
    info("  IMU cs=%i id=0x%02x xl=0b%08i g=0b%08i\n", cs, id, bin(xl), bin(g));
    if (id == 0x00) {
        warn("Gyro was not able to initialize\n");
        config_set_problem(PROBLEM_GYRO, true);
    }
}

void imu_init() {
    info("INIT: IMU\n");
    imu_channel_select();
    imu_load_calibration();
    imu_init_single(IMU0, IMU_CTRL2_G_500);
    imu_init_single(IMU1, IMU_CTRL2_G_125);
}

void imu_power_off_single(uint8_t cs) {
    bus_spi_write(cs, IMU_CTRL1_XL, IMU_CTRL1_XL_OFF);
    bus_spi_write(cs, IMU_CTRL2_G, IMU_CTRL2_G_OFF);
    uint8_t xl = bus_spi_read_one(cs, IMU_READ | IMU_CTRL1_XL);
    uint8_t g = bus_spi_read_one(cs, IMU_READ | IMU_CTRL2_G);
    info("IMU cs=%i xl=0b%08i g=0b%08i\n", cs, bin(xl), bin(g));
}

void imu_power_off() {
    imu_power_off_single(IMU0);
    imu_power_off_single(IMU1);
}

Vector imu_read_gyro_bits(uint8_t cs) {
    uint8_t buf[6];
    bus_spi_read(cs, IMU_READ | IMU_OUTX_L_G, buf, 6);

    int16_t y =  (int16_t)(((uint16_t)buf[1] << 8) | (uint16_t)buf[0]);
    int16_t z =  (int16_t)(((uint16_t)buf[3] << 8) | (uint16_t)buf[2]);
    int16_t x = -(int16_t)(((uint16_t)buf[5] << 8) | (uint16_t)buf[4]);

    float offset_x = (cs==PIN_SPI_CS0) ? imu_calib0.gyro[0].offset : imu_calib1.gyro[0].offset;
    float offset_y = (cs==PIN_SPI_CS0) ? imu_calib0.gyro[1].offset : imu_calib1.gyro[1].offset;
    float offset_z = (cs==PIN_SPI_CS0) ? imu_calib0.gyro[2].offset : imu_calib1.gyro[2].offset;

    #ifdef DEVICE_ALPAKKA_V0
        return (Vector){
            (float)x - offset_x,
            (float)y - offset_y,
            (float)z - offset_z,
        };
    #else /* DEVICE_ALPAKKA_V1 */
        return (Vector){
            (float)x - offset_x,
            -(float)y - offset_y,
            -(float)z - offset_z,
        };
    #endif
}

Vector imu_read_accel_bits(uint8_t cs) {
    uint8_t buf[6];
    bus_spi_read(cs, IMU_READ | IMU_OUTX_L_XL, buf, 6);

    int16_t x =  (int16_t)(((uint16_t)buf[1] << 8) + (uint16_t)buf[0]);
    int16_t y =  (int16_t)(((uint16_t)buf[3] << 8) + (uint16_t)buf[2]);
    int16_t z =  (int16_t)(((uint16_t)buf[5] << 8) + (uint16_t)buf[4]);

    float offset_x = (cs==PIN_SPI_CS0) ? offset_accel_0_x : offset_accel_1_x;
    float offset_y = (cs==PIN_SPI_CS0) ? offset_accel_0_y : offset_accel_1_y;
    float offset_z = (cs==PIN_SPI_CS0) ? offset_accel_0_z : offset_accel_1_z;

    #ifdef DEVICE_ALPAKKA_V0
        return (Vector){
            (float)x - offset_x,
            (float)y - offset_y,
            (float)z - offset_z,
        };
    #else /* DEVICE_ALPAKKA_V1 */
        return (Vector){
            -(float)x - offset_x,
            -(float)y - offset_y,
            (float)z - offset_z,
        };
    #endif
}

Vector imu_read_gyro_burst(uint8_t cs, uint8_t samples) {
    float x = 0;
    float y = 0;
    float z = 0;
    for(uint8_t i=0; i<samples; i++) {
        Vector sample = imu_read_gyro_bits(cs);
        x += sample.x;
        y += sample.y;
        z += sample.z;
    }
    x /= samples;
    y /= samples;
    z /= samples;
    return (Vector){x, y, z};
}

Vector imu_read_gyro() {
    Vector gyro0 = imu_read_gyro_bits(IMU0);
    Vector gyro1 = imu_read_gyro_bits(IMU1);
    Vector weight_0 = {
        imu_gyro0_weight[0],
        imu_gyro0_weight[1],
        imu_gyro0_weight[2]
    };

    // IMU 0 is on 500dps, IMU 1 is on 125dps.
    // when IMU 0 is on 20% of its range or higher, IMU 1 is on 80% of its range
    // then IMU 0 takes over 100% of the weight.
    // When vector-absolute value saturates, it is safer to switch all axes.
    
    if (gyro0.x*gyro0.x + gyro0.y*gyro0.y + gyro0.z*gyro0.z > 0.20f*0.20f*GYRO_MAXVAL_INT*GYRO_MAXVAL_INT) {
        weight_0.x = 1;
        weight_0.y = 1;
        weight_0.z = 1;
    }

    float x = (gyro0.x * weight_0.x) + (gyro1.x * (1.0f-weight_0.x) / 4.0f);
    float y = (gyro0.y * weight_0.y) + (gyro1.y * (1.0f-weight_0.y) / 4.0f);
    float z = (gyro0.z * weight_0.z) + (gyro1.z * (1.0f-weight_0.z) / 4.0f);
    return (Vector){x, y, z};
}

Vector imu_read_accel() {
    Vector accel0 = imu_read_accel_bits(IMU0);
    Vector accel1 = imu_read_accel_bits(IMU1);
    return (Vector){
        (accel0.x + accel1.x) / 2,
        (accel0.y + accel1.y) / 2,
        (accel0.z + accel1.z) / 2
    };
}

void imu_calibrate_single(uint8_t cs, ImuCalib *calib) {
    char *mode_str = "both";
    info("IMU: cs=%i calibrating %s...\n", cs, mode_str);
    float mean_x_accel = 0;
    float mean_x_gyro = 0;
    float mean_y_accel = 0;
    float mean_y_gyro = 0;
    float mean_z_accel = 0;
    float mean_z_gyro = 0;
    float  var_x_accel = 0;
    float  var_x_gyro = 0;
    float  var_y_accel = 0;
    float  var_y_gyro = 0;
    float  var_z_accel = 0;
    float  var_z_gyro = 0;
    // Determine number of samples.
    uint32_t nsamples;
    nsamples = CFG_CALIBRATION_SAMPLES_GYRO;
    Config *config = config_read();
    if (config->long_calibration) {
        nsamples *= CFG_CALIBRATION_LONG_FACTOR;

    }
    // Sampling.
    uint32_t i = 0;
    info("| 0%%%*s100%% |\n", CFG_CALIBRATION_PROGRESS_BAR - 10, "");
    while(i < nsamples) {
        Vector sample_accel = imu_read_accel_bits(cs);
        Vector sample_gyro = imu_read_gyro_bits(cs);
        
        // using cumulative average algorithm
        // the reason is that this way variance can use the actual value of the average

        mean_x_accel = (sample_accel.x + mean_x_accel*i)/(i+1);
        mean_y_accel = (sample_accel.y + mean_y_accel*i)/(i+1);
        mean_z_accel = (sample_accel.z + mean_z_accel*i)/(i+1);
        var_x_accel = ((sample_accel.x - mean_x_accel) * (sample_accel.x - mean_x_accel) + var_x_accel * i) / (i + 1);
        var_y_accel = ((sample_accel.y - mean_y_accel) * (sample_accel.y - mean_y_accel) + var_y_accel * i) / (i + 1);
        var_z_accel = ((sample_accel.z - mean_z_accel) * (sample_accel.z - mean_z_accel) + var_z_accel * i) / (i + 1);

        mean_x_gyro = (sample_gyro.x + mean_x_gyro*i)/(i+1);
        mean_y_gyro = (sample_gyro.y + mean_y_gyro*i)/(i+1);
        mean_z_gyro = (sample_gyro.z + mean_z_gyro*i)/(i+1);
        var_x_gyro = ((sample_gyro.x - mean_x_gyro) * (sample_gyro.x - mean_x_gyro) + var_x_gyro * i) / (i + 1);
        var_y_gyro = ((sample_gyro.y - mean_y_gyro) * (sample_gyro.y - mean_y_gyro) + var_y_gyro * i) / (i + 1);
        var_z_gyro = ((sample_gyro.z - mean_z_gyro) * (sample_gyro.z - mean_z_gyro) + var_z_gyro * i) / (i + 1);

        i++;
        sleep_us(1E6 / 6660); // 6660Hz sampling rate of IMU.
        if (!(i % (nsamples / CFG_CALIBRATION_PROGRESS_BAR))) info("=");
    }
    calib->accel[0].offset = mean_x_accel;
    calib->accel[1].offset = mean_y_accel;
    calib->accel[2].offset = mean_z_accel;
    calib-> gyro[0].offset = mean_x_gyro;
    calib-> gyro[1].offset = mean_y_gyro;
    calib-> gyro[2].offset = mean_z_gyro;

    calib->accel[0].variance = var_x_accel;
    calib->accel[1].variance = var_y_accel;
    calib->accel[2].variance = var_z_accel;
    calib-> gyro[0].variance = var_x_gyro;
    calib-> gyro[1].variance = var_y_gyro;
    calib-> gyro[2].variance = var_z_gyro;

    calib->accel[0].stddev = sqrtf(var_x_accel);
    calib->accel[1].stddev = sqrtf(var_y_accel);
    calib->accel[2].stddev = sqrtf(var_z_accel);
    calib-> gyro[0].stddev = sqrtf(var_x_gyro);
    calib-> gyro[1].stddev = sqrtf(var_y_gyro);
    calib-> gyro[2].stddev = sqrtf(var_z_gyro);



    // Assuming the resting state of the controller is having a vector of 1G
    // pointing down. (Newton's fault for inventing the gravity /jk).

    info("\nIMU: cs=%i %s calibrated \n x_acc=%.02f y_acc=%.02f z_acc=%.02f\n", cs, mode_str, 
        calib->accel[0].offset, calib->accel[1].offset, calib->accel[2].offset);
    info(" x_gyro=%.02f y_gyro=%.02f z_gyro=%.02f\n",
        calib->gyro[0].offset, calib->gyro[1].offset, calib->gyro[2].offset);
    info(" x_acc_var=%.02f y_acc_var=%.02f z_acc_var=%.02f\n",
        calib->accel[0].variance, calib->accel[1].variance, calib->accel[2].variance);
    info(" x_gyro_var=%.02f y_gyro_var=%.02f z_gyro_var=%.02f\n",
        calib->gyro[0].variance, calib->gyro[1].variance, calib->gyro[2].variance);
}

void imu_load_calibration() {
    Config *config = config_read();
    imu_calib0.gyro[0].offset = config->offset_gyro_0_x - (config->offset_gyro_user_x * GYRO_USER_OFFSET_FACTOR);
    imu_calib0.gyro[1].offset = config->offset_gyro_0_y - (config->offset_gyro_user_y * GYRO_USER_OFFSET_FACTOR);
    imu_calib0.gyro[2].offset = config->offset_gyro_0_z - (config->offset_gyro_user_z * GYRO_USER_OFFSET_FACTOR);
    imu_calib1.gyro[0].offset = config->offset_gyro_1_x - (config->offset_gyro_user_x * GYRO_USER_OFFSET_FACTOR);
    imu_calib1.gyro[1].offset = config->offset_gyro_1_y - (config->offset_gyro_user_y * GYRO_USER_OFFSET_FACTOR);
    imu_calib1.gyro[2].offset = config->offset_gyro_1_z - (config->offset_gyro_user_z * GYRO_USER_OFFSET_FACTOR);
    imu_calib0.accel[0].offset = config->offset_accel_0_x;
    imu_calib0.accel[1].offset = config->offset_accel_0_y;
    imu_calib0.accel[2].offset = config->offset_accel_0_z;
    imu_calib1.accel[0].offset = config->offset_accel_1_x;
    imu_calib1.accel[1].offset = config->offset_accel_1_y;
    imu_calib1.accel[2].offset = config->offset_accel_1_z;
    imu_calib0.gyro[0].variance = config->stddev_gyro_0_x * config->stddev_gyro_0_x;
    imu_calib0.gyro[1].variance = config->stddev_gyro_0_y * config->stddev_gyro_0_y;
    imu_calib0.gyro[2].variance = config->stddev_gyro_0_z * config->stddev_gyro_0_z;
    imu_calib1.gyro[0].variance = config->stddev_gyro_1_x * config->stddev_gyro_1_x;
    imu_calib1.gyro[1].variance = config->stddev_gyro_1_y * config->stddev_gyro_1_y;
    imu_calib1.gyro[2].variance = config->stddev_gyro_1_z * config->stddev_gyro_1_z;
    imu_calib0.accel[0].variance = config->stddev_accel_0_x * config->stddev_accel_0_x;
    imu_calib0.accel[1].variance = config->stddev_accel_0_y * config->stddev_accel_0_y;
    imu_calib0.accel[2].variance = config->stddev_accel_0_z * config->stddev_accel_0_z;
    imu_calib1.accel[0].variance = config->stddev_accel_1_x * config->stddev_accel_1_x;
    imu_calib1.accel[1].variance = config->stddev_accel_1_y * config->stddev_accel_1_y;
    imu_calib1.accel[2].variance = config->stddev_accel_1_z * config->stddev_accel_1_z;
    imu_calib0.gyro[0].stddev = config->stddev_gyro_0_x;
    imu_calib0.gyro[1].stddev = config->stddev_gyro_0_y;
    imu_calib0.gyro[2].stddev = config->stddev_gyro_0_z;
    imu_calib1.gyro[0].stddev = config->stddev_gyro_1_x;
    imu_calib1.gyro[1].stddev = config->stddev_gyro_1_y;
    imu_calib1.gyro[2].stddev = config->stddev_gyro_1_z;
    imu_calib0.accel[0].stddev = config->stddev_accel_0_x;
    imu_calib0.accel[1].stddev = config->stddev_accel_0_y;
    imu_calib0.accel[2].stddev = config->stddev_accel_0_z;
    imu_calib1.accel[0].stddev = config->stddev_accel_1_x;
    imu_calib1.accel[1].stddev = config->stddev_accel_1_y;
    imu_calib1.accel[2].stddev = config->stddev_accel_1_z;

    // IMU0 is on 500dps, IMU1 is on 125dps.

    // sum of variances
    Vector variance_div = {
        imu_calib0.gyro[0].variance + imu_calib1.gyro[0].variance/16,
        imu_calib0.gyro[1].variance + imu_calib1.gyro[1].variance/16,
        imu_calib0.gyro[2].variance + imu_calib1.gyro[2].variance/16
    };
    // weight of IMU0 proportional to variance of IMU1
    // the more noise on IMU1 the more we use IMU0...
    // see also: Kalman Filter.
    
    imu_gyro0_weight[0] = imu_calib1.gyro[0].variance/ variance_div.x/16;
    imu_gyro0_weight[1] = imu_calib1.gyro[1].variance/ variance_div.y/16;
    imu_gyro0_weight[2] = imu_calib1.gyro[2].variance/ variance_div.z/16;
    
    imu_gyro1_weight[0] = 1 - imu_gyro0_weight[0];
    imu_gyro1_weight[1] = 1 - imu_gyro0_weight[1];
    imu_gyro1_weight[2] = 1 - imu_gyro0_weight[2];
    info("IMU0 weight: x=%.02f y=%.02f z=%.02f\n", 
        imu_gyro0_weight[0], imu_gyro0_weight[1], imu_gyro0_weight[2]);
    info("IMU1 weight: x=%.02f y=%.02f z=%.02f\n", 
        imu_gyro1_weight[0], imu_gyro1_weight[1], imu_gyro1_weight[2]);
    
}

void imu_reset_calibration() {
    imu_calib0.gyro[0].offset = 0;
    imu_calib0.gyro[1].offset = 0;
    imu_calib0.gyro[2].offset = 0;
    imu_calib1.gyro[0].offset = 0;
    imu_calib1.gyro[1].offset = 0;
    imu_calib1.gyro[2].offset = 0;
    imu_calib0.accel[0].offset = 0;
    imu_calib0.accel[1].offset = 0;
    imu_calib0.accel[2].offset = 0;
    imu_calib1.accel[0].offset = 0;
    imu_calib1.accel[1].offset = 0;
    imu_calib1.accel[2].offset = 0;

    imu_calib0.gyro[0].variance = 1;
    imu_calib0.gyro[1].variance = 1;
    imu_calib0.gyro[2].variance = 1;
    imu_calib0.gyro[0].stddev = 1;
    imu_calib0.gyro[1].stddev = 1;
    imu_calib0.gyro[2].stddev = 1;
    imu_calib1.gyro[0].variance = 1;
    imu_calib1.gyro[1].variance = 1;
    imu_calib1.gyro[2].variance = 1;
    imu_calib1.gyro[0].stddev = 1;
    imu_calib1.gyro[1].stddev = 1;
    imu_calib1.gyro[2].stddev = 1;
    imu_calib0.accel[0].variance = 1;
    imu_calib0.accel[1].variance = 1;
    imu_calib0.accel[2].variance = 1;
    imu_calib1.accel[0].variance = 1;
    imu_calib1.accel[1].variance = 1;
    imu_calib1.accel[2].variance = 1;
    imu_calib0.accel[0].stddev = 1;
    imu_calib0.accel[1].stddev = 1;
    imu_calib0.accel[2].stddev = 1;
    imu_calib1.accel[0].stddev = 1;
    imu_calib1.accel[1].stddev = 1;
    imu_calib1.accel[2].stddev = 1;
}

void imu_calibrate() {
    config_set_gyro_user_offset(0, 0, 0);
    imu_reset_calibration();
    imu_calibrate_single(PIN_SPI_CS0, &imu_calib0);
    imu_calibrate_single(PIN_SPI_CS1, &imu_calib1);

    config_set_gyro_offset(
        imu_calib0.gyro[0].offset,
        imu_calib0.gyro[1].offset,
        imu_calib0.gyro[2].offset,
        imu_calib1.gyro[0].offset,
        imu_calib1.gyro[1].offset,
        imu_calib1.gyro[2].offset
    );
    config_set_accel_offset(
        imu_calib0.accel[0].offset,
        imu_calib0.accel[1].offset,
        imu_calib0.accel[2].offset,
        imu_calib1.accel[0].offset,
        imu_calib1.accel[1].offset,
        imu_calib1.accel[2].offset
    );
    config_set_gyro_stddev(
        imu_calib0.gyro[0].stddev,
        imu_calib0.gyro[1].stddev,
        imu_calib0.gyro[2].stddev,
        imu_calib1.gyro[0].stddev,
        imu_calib1.gyro[1].stddev,
        imu_calib1.gyro[2].stddev
    );
    config_set_accel_stddev(
        imu_calib0.accel[0].stddev,
        imu_calib0.accel[1].stddev,
        imu_calib0.accel[2].stddev,
        imu_calib1.accel[0].stddev,
        imu_calib1.accel[1].stddev,
        imu_calib1.accel[2].stddev
    );
    imu_load_calibration();
}
