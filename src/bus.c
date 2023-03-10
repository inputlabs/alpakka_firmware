// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include <hardware/gpio.h>
#include <hardware/i2c.h>
#include <hardware/spi.h>
#include "bus.h"
#include "pin.h"
#include "helper.h"

void bus_i2c_init() {
    printf("INIT: I2C bus\n");
    i2c_init(i2c1, I2C_FREQ);
    gpio_set_function(PIN_SDA, GPIO_FUNC_I2C);
    gpio_set_function(PIN_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(PIN_SDA);
    gpio_pull_up(PIN_SCL);
    if (!gpio_get(PIN_SDA) || !gpio_get(PIN_SCL)) {
        printf("ERROR: I2C bus is not clean\n");
        sleep_ms(1000000000);
    }
}

int8_t bus_i2c_acknowledge(uint8_t device) {
    uint8_t buf = 0;
    return i2c_read_blocking(i2c1, device, &buf, 1, false);
}

void bus_i2c_read(uint8_t device, uint8_t reg, uint8_t *buf, uint8_t len) {
    i2c_write_blocking(i2c1, device, &reg, 1, true);
    i2c_read_blocking(i2c1, device, buf, len, false);
}

uint8_t bus_i2c_read_one(uint8_t device, uint8_t reg) {
    uint8_t buf[1] = {0};
    bus_i2c_read(device, reg, buf, 1);
    return buf[0];
}

uint16_t bus_i2c_read_two(uint8_t device, uint8_t reg) {
    uint16_t buf[1] = {0};
    bus_i2c_read(device, reg, (uint8_t*)&buf, 2);
    return buf[0];
}

void bus_i2c_write(uint8_t device, uint8_t reg, uint8_t value) {
    uint8_t data[] = {reg, value};
    i2c_write_blocking(i2c1, device, data, 2, false);
}

void bus_i2c_io_init() {
    printf("INIT: I2C IO\n");
    bus_i2c_write(I2C_IO_0, I2C_IO_REG_POLARITY,   0b11111111);
    bus_i2c_write(I2C_IO_0, I2C_IO_REG_POLARITY+1, 0b11111111);
    bus_i2c_write(I2C_IO_1, I2C_IO_REG_POLARITY,   0b11111111);
    bus_i2c_write(I2C_IO_1, I2C_IO_REG_POLARITY+1, 0b11111111);
    bus_i2c_write(I2C_IO_0, I2C_IO_REG_PULLUP,     0b11111111);
    bus_i2c_write(I2C_IO_0, I2C_IO_REG_PULLUP+1,   0b11111111);
    bus_i2c_write(I2C_IO_1, I2C_IO_REG_PULLUP,     0b11111111);
    bus_i2c_write(I2C_IO_1, I2C_IO_REG_PULLUP+1,   0b11111111);
    printf("  I2C_IO_0 ");
    printf("ack=%i ", bus_i2c_acknowledge(I2C_IO_0));
    printf("polarity=%i ", bin(bus_i2c_read_one(I2C_IO_0, I2C_IO_REG_POLARITY)));
    printf("pullup=%i\n", bin(bus_i2c_read_one(I2C_IO_0, I2C_IO_REG_PULLUP)));
    printf("  I2C_IO_1 ");
    printf("ack=%i ", bus_i2c_acknowledge(I2C_IO_1));
    printf("polarity=%i ", bin(bus_i2c_read_one(I2C_IO_1, I2C_IO_REG_POLARITY)));
    printf("pullup=%i\n", bin(bus_i2c_read_one(I2C_IO_1, I2C_IO_REG_PULLUP)));
}

uint16_t io_cache_0;
uint16_t io_cache_1;

void bus_i2c_io_update_cache() {
    io_cache_0 = bus_i2c_read_two(I2C_IO_0, I2C_IO_REG_INPUT);
    io_cache_1 = bus_i2c_read_two(I2C_IO_1, I2C_IO_REG_INPUT);
}

uint16_t bus_i2c_io_get_cache(uint8_t index) {
    return index ? io_cache_1 : io_cache_0;
}

void bus_spi_init() {
    printf("INIT: SPI bus\n");
    spi_init(spi1, SPI_FREQ);
    gpio_set_function(PIN_SPI_CK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SPI_TX, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SPI_RX, GPIO_FUNC_SPI);
    gpio_init(PIN_SPI_CS0);
    gpio_init(PIN_SPI_CS1);
    gpio_set_dir(PIN_SPI_CS0, GPIO_OUT);
    gpio_set_dir(PIN_SPI_CS1, GPIO_OUT);
    gpio_put(PIN_SPI_CS0, true);
    gpio_put(PIN_SPI_CS1, true);
}

void bus_spi_write(uint8_t cs, uint8_t reg, uint8_t value) {
    gpio_put(cs, false);
    uint8_t buf[] = {reg, value};
    spi_write_blocking(spi1, buf, 2);
    gpio_put(cs, true);
}

void bus_spi_read(uint8_t cs, uint8_t reg, uint8_t *buf, uint8_t size) {
    gpio_put(cs, false);
    reg |= 0b10000000;  // Read byte.
    spi_write_blocking(spi1, &reg, 1);
    spi_read_blocking(spi1, 0, buf, size);
    gpio_put(cs, true);
}

uint8_t bus_spi_read_one(uint8_t cs, uint8_t reg) {
    uint8_t buf[1] = {0};
    bus_spi_read(cs, reg, buf, 1);
    return buf[0];
}

void bus_init() {
    bus_i2c_init();
    bus_i2c_io_init();
    bus_spi_init();
}
