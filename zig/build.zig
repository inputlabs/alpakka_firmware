const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const release_small = b.option(bool, "release-small", "Build with ReleaseSmall optimization") orelse false;
    const optimize: std.builtin.OptimizeMode = if (release_small) .ReleaseSmall else .Debug;

    const lib = b.addStaticLibrary(.{
        .name = "alpakka_zig",
        .root_source_file = b.path("src/lib.zig"),
        .target = target,
        .optimize = optimize,
    });

    b.installArtifact(lib);

    const unit_tests = b.addTest(.{
        .root_source_file = b.path("src/lib.zig"),
        .target = target,
        .optimize = optimize,
    });
    const run_tests = b.addRunArtifact(unit_tests);
    const test_step = b.step("test", "Run Zig unit tests");
    test_step.dependOn(&run_tests.step);
} 