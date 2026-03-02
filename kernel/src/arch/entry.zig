//! x86_64 Kernel Entry Point (Assembly)
//! Architecture: x86_64
//! Safety: Bootloader must set up 64-bit paging and leave CPU in long mode

const std = @import("std");

// Entry point defined in assembly
// Bootloader jumps here with:
// - 64-bit paging enabled
// - CPU in long mode
// - Stack set up
// - rdi = pointer to BootInfo structure
export var entry linksection(".text.entry") = @import("../../main.zig").kinit;
