const std = @import("std");

pub fn build(b: *std.Build) void {
    // Target RP2040 ARM.
    const optimize = b.standardOptimizeOption(.{});
    const target = b.resolveTargetQuery(.{
        .abi = .eabi,
        .cpu_arch = .thumb,
        .os_tag = .freestanding,
        .cpu_model = .{
            .explicit = &std.Target.arm.cpu.cortex_m0plus
        },
    });

    // Create library.
    const lib = b.addStaticLibrary(.{
        .name = "zigpakka",
        .root_source_file = b.path("src-zig/zigpakka.zig"),
        .target = target,
        .optimize = optimize,
    });
    b.installArtifact(lib);

    // Compile Pico-SDK with Cmake.
    const picosdk = b.addSystemCommand(&.{ "make"});
    picosdk.setEnvironmentVariable("DEVICE", "dongle");
    picosdk.setName("pico-sdk");
    picosdk.has_side_effects = true;
    // _ = picosdk.captureStdOut();
    // _ = picosdk.captureStdErr();
    b.getInstallStep().dependOn(&picosdk.step);
}
