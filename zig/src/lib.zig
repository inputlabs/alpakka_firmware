const std = @import("std");

pub const Vector = extern struct {
    x: f64,
    y: f64,
    z: f64,
};

pub export fn zig_vector_normalize(v: Vector) callconv(.C) Vector {
    const mag2: f64 = v.x * v.x + v.y * v.y + v.z * v.z;
    const diff: f64 = @abs(mag2 - 1.0);
    if (diff > 0.0001) {
        const mag: f64 = @sqrt(mag2);
        return .{ .x = v.x / mag, .y = v.y / mag, .z = v.z / mag };
    }
    return v;
}

test "zig_vector_normalize makes unit length (3,4,0) -> (0.6,0.8,0)" {
    const v = Vector{ .x = 3.0, .y = 4.0, .z = 0.0 };
    const n = zig_vector_normalize(v);
    try std.testing.expect(std.math.approxEqAbs(f64, n.x, 0.6, 1e-10));
    try std.testing.expect(std.math.approxEqAbs(f64, n.y, 0.8, 1e-10));
    try std.testing.expect(std.math.approxEqAbs(f64, n.z, 0.0, 1e-10));
    const len = @sqrt(n.x * n.x + n.y * n.y + n.z * n.z);
    try std.testing.expect(std.math.approxEqAbs(f64, len, 1.0, 1e-12));
}

test "zig_vector_normalize returns same vector if already unit" {
    const v = Vector{ .x = 1.0, .y = 0.0, .z = 0.0 };
    const n = zig_vector_normalize(v);
    try std.testing.expect(std.math.approxEqAbs(f64, n.x, 1.0, 0.0));
    try std.testing.expect(std.math.approxEqAbs(f64, n.y, 0.0, 0.0));
    try std.testing.expect(std.math.approxEqAbs(f64, n.z, 0.0, 0.0));
} 