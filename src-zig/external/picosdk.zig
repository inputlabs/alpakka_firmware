// @ arm-toolchain/arm-none-eabi/include/stdio.h
// int printf (const char *__restrict, ...);
pub extern fn printf(format: [*:0]const u8, ...) i16;

// @ pico-sdk/src/rp2_common/hardware_i2c/include/hardware/i2c.h
pub extern var _i2c1: *anyopaque;

// @ pico-sdk/src/rp2_common/hardware_i2c/include/hardware/i2c.h
// int i2c_read_blocking(i2c_inst_t *i2c, uint8_t addr, uint8_t *dst, size_t len, bool nostop);
pub extern fn i2c_read_blocking(i2c:*anyopaque, addr:u8, dst:*u8, len:usize, nostop:bool) i16;

// @ pico-sdk/src/rp2_common/hardware_i2c/include/hardware/i2c.h
// int i2c_write_blocking(i2c_inst_t *i2c, uint8_t addr, const uint8_t *src, size_t len, bool nostop);
pub extern fn i2c_write_blocking(i2c:*anyopaque, addr:u8, src:*const u8, len:usize, nostop:bool) i16;
