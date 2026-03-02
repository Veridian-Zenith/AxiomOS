//! Kernel build configuration
//! Architecture: x86_64
//! Targets: x86_64-freestanding kernel

const std = @import("std");

pub fn build(b: *std.Build) void {
    const optimize = b.standardOptimizeOption(.{});

    const kernel_exe = b.addExecutable(.{
        .name = "kernel",
        .optimize = optimize,
    });
    kernel_exe.root_module.root_source_file = b.path("src/main.zig");
    kernel_exe.root_module.target = b.resolveTargetQuery(.{
        .cpu_arch = .x86_64,
        .os_tag = .freestanding,
        .abi = .none,
    });

    kernel_exe.setLinkerScript(b.path("linker.ld"));
    kernel_exe.link_function_sections = true;
    kernel_exe.link_data_sections = true;

    const kernel_install = b.addInstallArtifact(kernel_exe, .{});
    b.getInstallStep().dependOn(&kernel_install.step);
}
