//! Main AxiomOS build configuration
//! Architecture: x86_64
//! Targets: UEFI bootloader + x86_64-freestanding kernel

const std = @import("std");

pub fn build(b: *std.Build) void {
    const optimize = b.standardOptimizeOption(.{});

    // ====================================================================
    // Bootloader Target (x86_64 UEFI)
    // ====================================================================
    const bootloader_exe = b.addExecutable(.{
        .name = "bootloader",
        .root_module = b.createModule(.{
            .root_source_file = b.path("bootloader/main.zig"),
            .target = b.resolveTargetQuery(.{
                .cpu_arch = .x86_64,
                .os_tag = .uefi,
                .abi = .msvc,
            }),
            .optimize = optimize,
        }),
    });
    bootloader_exe.entry = .{ .symbol_name = "efi_main" };
    // subsystem removed - UEFI uses .uefi OS tag, no subsystem needed

    const bootloader_install = b.addInstallArtifact(bootloader_exe, .{});
    b.getInstallStep().dependOn(&bootloader_install.step);

    // ====================================================================
    // Kernel Target (x86_64 Freestanding)
    // ====================================================================
    const kernel_exe = b.addExecutable(.{
        .name = "kernel",
        .root_module = b.createModule(.{
            .root_source_file = b.path("kernel/src/main.zig"),
            .target = b.resolveTargetQuery(.{
                .cpu_arch = .x86_64,
                .os_tag = .freestanding,
                .abi = .none,
            }),
            .optimize = optimize,
        }),
    });

    kernel_exe.setLinkerScript(b.path("kernel/linker.ld"));
    kernel_exe.root_module.strip = optimize == .ReleaseSafe or optimize == .ReleaseSmall;
    kernel_exe.link_function_sections = true;
    kernel_exe.link_data_sections = true;

    const kernel_install = b.addInstallArtifact(kernel_exe, .{});
    b.getInstallStep().dependOn(&kernel_install.step);

    // Default step: build everything
    b.default_step.dependOn(&bootloader_install.step);
    b.default_step.dependOn(&kernel_install.step);
}
