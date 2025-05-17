// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#pragma once
#include <stdbool.h>
#include <stdint.h>

#define I2C_FREQ 400 * 1000  // Hz.
#define SPI_FREQ 10 * 1000 * 1000  // Hz.

// I2C IO expansion.
#define I2C_IO_ID 0b0100000
#define I2C_IO_0  I2C_IO_ID | 0b000
#define I2C_IO_1  I2C_IO_ID | 0b001
#define I2C_IO_REG_INPUT 0x00
#define I2C_IO_REG_OUTPUT 0x02
#define I2C_IO_REG_POLARITY 0x04
#define I2C_IO_REG_CONFIG 0x06
#define I2C_IO_REG_PULL 0x46
#define I2C_IO_REG_PULL_DIR 0x48

// Bus channels.
#if defined DEVICE_ALPAKKA_V0
    #define I2C_CHANNEL i2c1
    #define SPI_CHANNEL spi1
#elif defined DEVICE_HAS_MARMOTA
    #define I2C_CHANNEL i2c1
    #define SPI_CHANNEL spi0
#endif

typedef enum Tristate_enum {
    TRIESTATE_FLOAT,
    TRIESTATE_DOWN,
    TRIESTATE_UP,
} Tristate;

void bus_init();

// I2C.
int8_t bus_i2c_acknowledge(uint8_t device);
void bus_i2c_write(uint8_t device, uint8_t reg, uint8_t value);
void bus_i2c_read(uint8_t device, uint8_t reg, uint8_t *buf, uint8_t len);
uint8_t bus_i2c_read_one(uint8_t device, uint8_t reg);
uint16_t bus_i2c_read_two(uint8_t device, uint8_t reg);

// IO expanders.
void bus_i2c_io_cache_update();
bool bus_i2c_io_cache_read(uint8_t device_index, uint8_t bit_index);
bool bus_i2c_io_read(uint8_t device_id, uint8_t bit_index);

// SPI.
void bus_spi_read(uint8_t cs, uint8_t reg, uint8_t *buf, uint8_t size);
void bus_spi_write_32(uint8_t cs, uint8_t reg, uint8_t buf[32]);
uint8_t bus_spi_read_one(uint8_t cs, uint8_t reg);
void bus_spi_write(uint8_t cs, uint8_t reg, uint8_t value);
