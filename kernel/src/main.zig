//! AxiomOS Kernel Entry Point
//! Architecture: x86_64
//! Safety: Running in long mode (64-bit), paging enabled

const std = @import("std");
const builtin = @import("builtin");

const arch = @import("arch/x86_64.zig");
const serial = @import("serial.zig");
const memory = @import("mm/allocator.zig");

// Import bootloader info
pub const BootInfo = extern struct {
    framebuffer_base: u64,
    framebuffer_size: u64,
    framebuffer_stride: u32,
    framebuffer_width: u32,
    framebuffer_height: u32,
    framebuffer_format: u32,
    efi_memory_map: [*]u8,
    efi_memory_map_size: usize,
    efi_descriptor_size: usize,
    efi_descriptor_version: u32,
    kernel_entry: u64,
};

/// Kernel panic - prints error and halts
pub fn panic(comptime format: []const u8, args: anytype) noreturn {
    serial.printf("KERNEL PANIC: " ++ format ++ "\n", args);
    arch.x86_64.halt();
}

/// Main kernel entry point
/// Parameters:
///   boot_info - Structure from bootloader with system info
/// Safety: CPU must be in 64-bit long mode with paging enabled
pub fn kinit(boot_info_ptr: *BootInfo) noreturn {
    // ====================================================================
    // Initialize Serial Output (UART) for debugging
    // ====================================================================
    serial.init();
    serial.puts("=====================================\n");
    serial.puts("[AxiomOS Kernel] Initializing...\n");
    serial.puts("=====================================\n");

    // ====================================================================
    // Set up Panic Handler
    // ====================================================================
    std.debug.panic = panic;

    // ====================================================================
    // Initialize x86_64 Architecture
    // ====================================================================
    serial.puts("[Kernel] Setting up GDT...\n");
    arch.x86_64.setupGdt();

    serial.puts("[Kernel] Setting up IDT...\n");
    arch.x86_64.setupIdt();

    serial.puts("[Kernel] Initializing PIC/APIC...\n");
    arch.x86_64.initPic();

    serial.puts("[Kernel] Setting up timer (HPET/PIT)...\n");
    arch.x86_64.initTimer();

    // ====================================================================
    // Initialize Memory Management
    // ====================================================================
    serial.printfln("Boot Framebuffer: 0x{x} (size: {} bytes)\n", boot_info_ptr.framebuffer_base, boot_info_ptr.framebuffer_size);
    serial.printfln("Boot Resolution: {}x{}\n", boot_info_ptr.framebuffer_width, boot_info_ptr.framebuffer_height);

    serial.puts("[Kernel] Parsing EFI memory map...\n");
    memory.parseEfiMemoryMap(
        @as([*]u8, @ptrFromInt(boot_info_ptr.efi_memory_map)),
        boot_info_ptr.efi_memory_map_size,
        boot_info_ptr.efi_descriptor_size,
    );

    serial.puts("[Kernel] Initializing physical memory allocator...\n");
    memory.initPhysicalAllocator();

    serial.puts("[Kernel] Setting up virtual memory (paging)...\n");
    memory.initVirtualMemory();

    // ====================================================================
    // Kernel Ready
    // ====================================================================
    serial.puts("\n");
    serial.puts("=====================================\n");
    serial.puts("[AxiomOS Kernel] Ready for userspace\n");
    serial.puts("=====================================\n");

    // TODO: Load first userspace process
    // TODO: Enable interrupts globally
    // For now, idle loop
    arch.x86_64.halt();
}

// ====================================================================
// Entry Point Stub (ASM linkage)
// ====================================================================

// The bootloader jumps to &kentry (always 0xffffffff80000000 in higher-half kernel)
// We define it in assembly to properly set up the environment
export var kentry linksection(".text.entry") = @import("arch/entry.zig").entry;
