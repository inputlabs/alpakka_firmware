// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Input Labs Oy.

#include <stdio.h>
#include <hardware/gpio.h>
#include <hardware/i2c.h>
#include <hardware/spi.h>
#include "bus.h"
#include "config.h"
#include "pin.h"
#include "helper.h"
#include "logging.h"

uint16_t io_cache_0;
uint16_t io_cache_1;

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

Tristate bus_i2c_io_tristate(uint8_t index) {
    bus_i2c_write(I2C_IO_0, I2C_IO_REG_PULL_DIR+1, 0b00000000);
    bool down = bus_i2c_io_read(I2C_IO_0, index);
    bus_i2c_write(I2C_IO_0, I2C_IO_REG_PULL_DIR+1, 0b11111111);
    bool up = bus_i2c_io_read(I2C_IO_0, index);
    if ( up !=  down) return TRIESTATE_FLOAT;
    if (!up && !down) return TRIESTATE_DOWN;
    if ( up &&  down) return TRIESTATE_UP;
}

void bus_i2c_io_pcb_gen_determine() {
    bus_i2c_write(I2C_IO_0, I2C_IO_REG_POLARITY+1, 0b00000000);
    bus_i2c_write(I2C_IO_0, I2C_IO_REG_PULL+1,     0b11111111);
    Tristate value_0 = bus_i2c_io_tristate(PIN_PCBGEN_0 - PIN_GROUP_IO_0);
    // NOTE: Use a ternary mask if versions go over 3.
    // See https://github.com/inputlabs/alpakka_pcb/blob/main/generations.md
    config_set_pcb_gen(value_0);
}

void bus_i2c_io_cache_update() {
    io_cache_0 = bus_i2c_read_two(I2C_IO_0, I2C_IO_REG_INPUT);
    io_cache_1 = bus_i2c_read_two(I2C_IO_1, I2C_IO_REG_INPUT);
}

bool bus_i2c_io_cache_read(uint8_t device_index, uint8_t bit_index) {
    return (device_index ? io_cache_1 : io_cache_0) & (1 << bit_index);
}

bool bus_i2c_io_read(uint8_t device_id, uint8_t bit_index) {
    uint16_t value = bus_i2c_read_two(device_id, I2C_IO_REG_INPUT);
    return value & (1 << bit_index);
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

void bus_i2c_init() {
    info("INIT: I2C bus\n");
    i2c_init(i2c1, I2C_FREQ);
    gpio_set_function(PIN_SDA, GPIO_FUNC_I2C);
    gpio_set_function(PIN_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(PIN_SDA);
    gpio_pull_up(PIN_SCL);
    if (!gpio_get(PIN_SDA) || !gpio_get(PIN_SCL)) {
        info("ERROR: I2C bus is not clean, unplug the controller\n");
        sleep_ms(1000000000);
    }
}

void bus_i2c_io_init_single(uint8_t id) {
    bus_i2c_write(id, I2C_IO_REG_POLARITY,   0b11111111);
    bus_i2c_write(id, I2C_IO_REG_POLARITY+1, 0b11111111);
    bus_i2c_write(id, I2C_IO_REG_PULL,   0b11111111);
    bus_i2c_write(id, I2C_IO_REG_PULL+1, 0b11111111);
    info("  IO id=%i ", id);
    info("ack=%i ", bus_i2c_acknowledge(id));
    info("polarity=%i ", bin(bus_i2c_read_one(id, I2C_IO_REG_POLARITY)));
    info("pull=%i\n", bin(bus_i2c_read_one(id, I2C_IO_REG_PULL)));
}

void bus_i2c_io_init() {
    info("INIT: I2C IO\n");
    bus_i2c_io_pcb_gen_determine();
    info("  PCB GEN: gen-%i\n", config_get_pcb_gen());
    bus_i2c_io_init_single(I2C_IO_0);
    bus_i2c_io_init_single(I2C_IO_1);
}

void bus_spi_init() {
    info("INIT: SPI bus\n");
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

void bus_init() {
    bus_i2c_init();
    bus_i2c_io_init();
    bus_spi_init();
}
