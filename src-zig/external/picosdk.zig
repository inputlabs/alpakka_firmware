pub extern fn printf(format: [*:0]const u8, ...) i16;

pub extern var _i2c1: *anyopaque;

pub extern fn i2c_read_blocking(
    i2c: *anyopaque,
    addr: u8,
    dst: *u8,
    len: usize,
    nostop: bool,
) i16;

pub extern fn i2c_write_blocking(
    i2c: *anyopaque,
    addr: u8,
    src: *const u8,
    len: usize,
    nostop: bool,
) i16;
