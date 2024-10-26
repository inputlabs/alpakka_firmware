const rp = @import("external/picosdk.zig");
const info = @import("external/c_logging.zig").info;

export fn zigadd(a:i8, b:i8) i8 {
    return a + b;
}

export fn zighello() void {
    // _ = rp.printf("Hello miau\n");
    info("Hello miau\n");
}

export fn bus_i2c_read(
    device: u8,
    reg: u8,
    buf: *u8,
    len: u8,
) void {
    _ = rp.i2c_write_blocking(rp._i2c1, device, &reg, 1, true);
    _ = rp.i2c_read_blocking(rp._i2c1, device, buf, len, false);
}
